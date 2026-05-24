#ifndef BASIC_STEPGEN_H
#define BASIC_STEPGEN_H

#include <cstdint>
#include <cmath>
#include <string>

#include "../../remora.h"
#include "../module.h"

// A DDS accumulator which can also be queried for how many cycles until the
// next trigger.  When triggered, it stops advancing and stays triggered, which
// is used to wait out dirhold and dirsetup and such.
template<int32_t CycleFrequency>
struct BasicDDSAccumulator
{
    int32_t value;

    inline BasicDDSAccumulator() : value(0) {}

    static constexpr int32_t low  = -CycleFrequency + 1;
    static constexpr int32_t high = CycleFrequency;

    inline bool triggered() const                     { return value < low || value > high; }
    inline void advance(int32_t freq, int32_t cycles) { if (!triggered()) value += 2*freq*cycles; }

    inline void resetTrigger()
    {
        assert(triggered());
        if (value < low) value += 2*CycleFrequency;
        else if (value > high) value -= 2*CycleFrequency;
        assert(!triggered());
    }

    inline int32_t cyclesUntilTrigger(int32_t freq) const
    {
        if (triggered()) return 0;
        if (freq == 0) return INT32_MAX;
        return 1 + ((freq > 0 ? high : low) - value) / freq / 2;
    }
};

// CycleCounter is used to track expiration time for various events like
// dirhold and dirsetup.  It wraps every thirty-some seconds, but is
// modelled after Linux jiffies, so that it will work correctly when it
// wraps.
//
// It doesn't own the time, which is why now is passed to start() and
// remaining(), but this value must be monotonically increasing.
//
// We must call update() periodically to disarm expired timers to prevent
// them from being active again when our time wraps.
class CycleCounter
{
private:
    uint32_t duration;
    uint32_t expiry;
    bool armed;

public:
    inline CycleCounter(uint32_t duration)
        : duration(duration)
        , expiry(0)
        , armed(false)
    {
    }

    inline void start(uint32_t now)
    {
        expiry = now + duration;
        armed = true;
    }

    inline uint32_t remaining(uint32_t now) const
    {
        return armed ? std::max(int32_t(0), int32_t(expiry - now)) : 0;
    }

    inline void disarmIfExpired(uint32_t now)
    {
        if (!remaining(now))
            armed = false;
    }
};

// A step generator which schedules pin changes in terms of cycles.  It tracks
// how far into the future it has scheduled, and schedules until at least until
// the next base thread tick so that it can keep the TX command queue full.
//
// For actual steps, the step high and low are scheduled right away, as nothing
// else can happen in between.  This can overschedule us a bit, but that's OK.
// For "evitable" events, we only schedule to the beginning of the next tick
// in case we receive new frequency commands in the mean time.
template<
    int32_t CpuFreq
  , int32_t ThreadFreq
  , class IOType
>
class BasicStepgen
  : public Module
  , protected IOType
{
    static_assert(CpuFreq % ThreadFreq == 0, "CpuFreq must be an integer multiple of ThreadFreq");
    static constexpr uint32_t cyclesPerTick = CpuFreq / ThreadFreq;

    static constexpr uint32_t nsToCycles(int32_t ns)
    {
        return ns > 0
            ? uint32_t((uint64_t(ns) * uint64_t(CpuFreq) + 999999999ULL) / 1000000000ULL)
            : cyclesPerTick;
    }

    using DDSAccumulator = BasicDDSAccumulator<CpuFreq>;

private:
    volatile int32_t rawCount;
    volatile int32_t frequency;
    int32_t localFrequency;
    DDSAccumulator dds;
    uint32_t tickEndCycle;
    uint32_t plannedCycles;
    uint32_t steplenCycles;
    int32_t maximumFrequency;
    CycleCounter dirsetup;
    CycleCounter dirhold;
    CycleCounter dirdelay;
    bool lastPulseWasForward;
    bool currentDirection;

public:
    BasicStepgen(
        std::string step,
        std::string direction,
        int32_t steplenNs,
        int32_t stepspaceNs,
        int32_t dirsetupNs,
        int32_t dirholdNs,
        int32_t dirdelayNs
    ) : IOType(step, direction)
      , rawCount(0)
      , frequency(0)
      , localFrequency(0)
      , tickEndCycle(0)
      , plannedCycles(0)
      , steplenCycles(nsToCycles(steplenNs))
      , maximumFrequency(CpuFreq / nsToCycles(steplenNs + stepspaceNs))
      , dirsetup(nsToCycles(dirsetupNs))
      , dirhold(nsToCycles(dirholdNs))
      , dirdelay(nsToCycles(dirdelayNs))
      , lastPulseWasForward(false)
      , currentDirection(false)
    {
        IOType::schedule(0, false, currentDirection);
    }

    // Callable from core0, owing to frequency volatility and it being the only
    // data member updated so it can't be inconsistent.
    void setFrequency(int32_t frequency, bool enabled)
    {
        if (!enabled)
        {
            this->frequency = 0;
            return;
        }
        this->frequency = frequency;
        if (std::abs(frequency) > maximumFrequency)
        {
            printf("frequency %d exceeds maximum %d\n", frequency, maximumFrequency);
        }
    }

    // Callable from core0, owing to rawCount volatility and it being the only
    // data member read so it can't be inconsistent.  Lagging a little bit is OK.
    int32_t getRawCount() const
    {
        return rawCount;
    }

    virtual void update() override
    {
        // Cache volatile frequency locally so it doesn't change during the
        // planning tick.
        localFrequency = frequency;

        tickEndCycle += cyclesPerTick;
        dirhold.disarmIfExpired(plannedCycles);
        dirsetup.disarmIfExpired(plannedCycles);
        dirdelay.disarmIfExpired(plannedCycles);

        if (localFrequency != 0)
        {
            planDirectionChange();
            planDirectionHolds();
            planSteps();
        }
        planWaitUntilEndOfTick();
    }

private:
    inline void planWaitThenSetPins(uint32_t cycles, bool step, bool dir)
    {
        uint32_t actual = IOType::schedule(cycles, step, dir);
        plannedCycles += actual;
        dds.advance(localFrequency, actual);
    }

    inline int32_t tickCyclesRemaining() const
    {
        return int32_t(tickEndCycle - plannedCycles);
    }

    inline void planWaitUntilEndOfTick()
    {
        int32_t toWait = tickCyclesRemaining();
        if (toWait > 0)
            planWaitThenSetPins(toWait, false, currentDirection);
    }

    // Wait for cycles, but not past the end of the tick in case we
    // get new orders in.
    inline void planEvitableWait(uint32_t cycles)
    {
        int32_t remaining = tickCyclesRemaining();
        if (remaining <= 0) return;
        planWaitThenSetPins(std::min(cycles, uint32_t(remaining)), false, currentDirection);
    }

    inline bool isForward() const
    {
        return localFrequency > 0;
    }

    inline void planDirectionChange()
    {
        if (currentDirection == isForward())
            return;

        uint32_t wait = dirhold.remaining(plannedCycles);
        if (wait > tickCyclesRemaining())
        {
            planWaitUntilEndOfTick();
            return;
        }

        currentDirection = isForward();
        planWaitThenSetPins(wait, false, currentDirection);
        dirsetup.start(plannedCycles);
    }

    inline void planDirectionHolds()
    {
        uint32_t dirwait = dirsetup.remaining(plannedCycles);
        if (isForward() != lastPulseWasForward)
            dirwait = std::max(dirwait, dirdelay.remaining(plannedCycles));
        if (dirwait > 0)
            planEvitableWait(dirwait);
    }

    inline void planSteps()
    {
        for (int32_t nextStep = dds.cyclesUntilTrigger(localFrequency);
             nextStep < tickCyclesRemaining();
             nextStep = dds.cyclesUntilTrigger(localFrequency))
            planStep(nextStep);
    }

    inline void planStep(int32_t nextStep)
    {
        planWaitThenSetPins(nextStep, true, currentDirection);
        dds.resetTrigger();
        if (isForward())
            ++this->rawCount;
        else
            --this->rawCount;
        lastPulseWasForward = isForward();
        dirdelay.start(plannedCycles);
        planWaitThenSetPins(steplenCycles, false, currentDirection);
        dirhold.start(plannedCycles);
    }
};

#endif
