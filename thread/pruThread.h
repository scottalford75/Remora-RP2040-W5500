#ifndef PRUTHREAD_H
#define PRUTHREAD_H

#include "../configuration.h"
#include "../modules/module.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/time.h"

#include <vector>
#include <cstdint>


template<int Pin>
struct DebugPin
{
    inline static void init(void)
    {
        gpio_init(Pin);
        gpio_set_dir(Pin, 1);
    }
    inline static void set(void)   { gpio_put(Pin, 1); }
    inline static void clear(void) { gpio_put(Pin, 0); }
};

template<irq_num_t Irq, uint32_t Period, class Thread>
struct IRQRunner
{
    static constexpr int slice = TIMER_ALARM_NUM_FROM_IRQ(Irq);
    static constexpr irq_num_t irq = Irq;
    static constexpr uint32_t period = Period;

    static void start(void)
    {
        hw_set_bits(&timer_hw->inte, 1u << slice);
        irq_set_exclusive_handler(Irq, handleInterrupt);
        irq_set_enabled(Irq, true);
        timer_hw->alarm[slice] = timer_hw->timerawl + Period;
    }

private:
    static void handleInterrupt(void)
    {
        hw_clear_bits(&timer_hw->intr, 1u << slice);
        timer_hw->alarm[slice] += Period;
        Thread::runModules();
    }
};

template<uint32_t Period, class Thread>
struct NonIRQRunner
{
    static constexpr uint32_t period = Period;
private:
    static uint32_t deadline;

public:
    static void start(void)
    {
        deadline = time_us_32() + Period;
    }

    static void run(void)
    {
        if (int32_t(deadline - time_us_32()) > 0) return;
        Thread::runModules();
        deadline += Period;
    }
};

template<uint32_t Period, class Thread>
uint32_t NonIRQRunner<Period, Thread>::deadline = 0;

template<class RunPolicy, class DebugPinPolicy>
class pruThread
    : public RunPolicy
{
    friend RunPolicy;
    static std::vector<Module*> modules;

protected:
    static void runModules(void)
    {
        DebugPinPolicy::set();
        for (auto& m : modules) m->runModule();
        DebugPinPolicy::clear();
    }

public:
    static void registerModule(Module *module)
    {
        modules.push_back(module);
    }

    static void start(void)
    {
        printf("    actual period = %u\n", RunPolicy::period);
        DebugPinPolicy::init();
        RunPolicy::start();
        printf("    timer started\n");
    }
};

template<class RunPolicy, class DebugPinPolicy>
std::vector<Module*> pruThread<RunPolicy, DebugPinPolicy>::modules;

struct BaseThread : public pruThread<IRQRunner<TIMER_IRQ_0, 1000000 / PRU_BASEFREQ, BaseThread>, DebugPin<6>> {};
struct ServoThread : public pruThread<NonIRQRunner<1000000 / PRU_SERVOFREQ, ServoThread>, DebugPin<27>> {};

#endif
