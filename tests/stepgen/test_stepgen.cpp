#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include "../../modules/stepgen/basic_stepgen.h"

// mini test harness

int failures = 0;

#define TEST(name) \
    void name##_impl_(void); \
    void name(void) \
    { \
        printf(#name "... "); \
        fflush(stdout); \
        int failures_on_entry = failures; \
        name##_impl_(); \
        if (failures == failures_on_entry) \
           printf("ok.\n"); \
        else \
           printf("\n\n"); \
    } \
    void name##_impl_(void)

// --- IO stub ---

struct TestIO
{
    struct Command
    {
        int tick;
        uint32_t cycles;
        bool step;
        bool dir;
    };

    struct StateChange
    {
        bool step, dir;
        uint32_t cycles;
    };

    std::vector<Command> commands;
    int tick = 0;

    TestIO(std::string, std::string) {}

    uint32_t schedule(uint32_t cycles, bool step, bool dir)
    {
        commands.push_back({tick, cycles, step, dir});
        return cycles;
    }

    std::vector<StateChange> recordedStateChanges() const
    {
        std::vector<StateChange> result;
        StateChange current{ false, false, 0 };
        for (auto const& c : commands)
        {
            current.cycles += c.cycles;
            if (current.step != c.step || current.dir != c.dir)
            {
                result.push_back(current);
                current = {c.step, c.dir, 0};
            }
        }
        result.push_back(current);
        return result;
    }
};

static const int32_t THREAD_FREQ = 40000;
static const char* STEP_PIN = "GP02";
static const char* DIR_PIN  = "GP03";

static const int32_t CPU_FREQ = 125000000;
static const uint32_t CYCLES_PER_TICK = CPU_FREQ / THREAD_FREQ;

constexpr uint32_t operator"" _ns(unsigned long long ns) { return uint64_t(CPU_FREQ)*ns/1000000000ULL; }
constexpr uint32_t operator"" _Hz(unsigned long long hz) { return CPU_FREQ/hz; }
constexpr uint32_t operator"" _KHz(unsigned long long khz) { return CPU_FREQ/(1000*khz); }
constexpr uint32_t operator"" _tick(unsigned long long t) { return t*CYCLES_PER_TICK; }

class TestStepgen : public BasicStepgen<CPU_FREQ, THREAD_FREQ, TestIO>
{
public:
    using BasicStepgen::BasicStepgen;
    TestIO& io() { return *this; }
};

class Scenario
{
private:
    int32_t steplen;
    int32_t stepspace;
    int32_t dirsetup;
    int32_t dirhold;
    int32_t dirdelay;
    bool reportedScheduleFailure;
    std::optional<TestStepgen> stepgen;

    void start()
    {
        if (!stepgen.has_value())
            stepgen.emplace(STEP_PIN, DIR_PIN, steplen, stepspace, dirsetup, dirhold, dirdelay);
    }

    void callUpdate()
    {
        auto& testIO = stepgen->io();
        size_t before = testIO.commands.size();
        stepgen->update();
        ++testIO.tick;
        uint64_t planned = 0;
        for (auto const& command : testIO.commands)
            planned += command.cycles;
        if (!reportedScheduleFailure && planned < uint64_t(testIO.tick)*CYCLES_PER_TICK)
        {
            reportedScheduleFailure = true;
            fail("update() did not plan enough cycles (planned %u, tick %u)\n", planned, uint64_t(testIO.tick)*CYCLES_PER_TICK);
        }
    }

    int countPulses(std::optional<bool> forward = {})
    {
        int n = 0;
        bool prevStep = false;
        for (auto const& c : stepgen->io().commands)
        {
            if (c.step && !prevStep)
                if (!forward.has_value() || *forward == c.dir)
                    ++n;
            prevStep = c.step;
        }
        return n;
    }

    void fail(const char *msg, ...)
    {
        va_list args;
        ++failures;
        printf("\n\n\e[31mFAILED\e[0m: ");
        va_start(args, msg);
        vprintf(msg, args);
        va_end(args);

        dumpCalls();
    }

public:
    Scenario()
      : steplen(0)
      , stepspace(0)
      , dirsetup(0)
      , dirhold(0)
      , dirdelay(0)
      , reportedScheduleFailure(false)
    {
    }

    void dumpCommands(decltype(stepgen->io().commands)::const_iterator begin, decltype(stepgen->io().commands)::const_iterator end)
    {
        for (auto it = begin; it != end; ++it)
        {
            auto const& cmd = *it;
            printf("    tick=%d cycles=%u step=%d dir=%d\n", cmd.tick, cmd.cycles, int(cmd.step), int(cmd.dir));
        }
    }

    Scenario& dumpCalls(int limit = 20)
    {
        auto const& calls = stepgen->io().commands;
        printf("\n  Calls (of %d, CYCLES_PER_TICK is %d):\n", int(calls.size()), int(CYCLES_PER_TICK));
        if (2*limit >= calls.size())
        {
            dumpCommands(calls.begin(), calls.end());
        }
        else
        {
            dumpCommands(calls.begin(), calls.begin()+limit);
            printf("         ...\n");
            dumpCommands(calls.end()-limit, calls.end());
        }
        return *this;
    }

    Scenario& withSteplen(int32_t value) { steplen = value; return *this; }
    Scenario& withStepspace(int32_t value) { stepspace = value; return *this; }
    Scenario& withDirsetup(int32_t value) { dirsetup = value; return *this; }
    Scenario& withDirhold(int32_t value) { dirhold = value; return *this; }
    Scenario& withDirdelay(int32_t value) { dirdelay = value; return *this; }

    Scenario& withFrequency(int32_t frequency, bool enabled = true)
    {
        start();
        stepgen->setFrequency(frequency, enabled);
        return *this;
    }

    Scenario& afterRunning1Second()
    {
        start();
        for (int i = 0; i < THREAD_FREQ; i++)
            callUpdate();
        return *this;
    }

    Scenario& afterPulses(int n)
    {
        start();
        auto& testIO = stepgen->io();
        int seen = 0;
        for (int i = 0; i < THREAD_FREQ; ++i)
        {
            size_t before = testIO.commands.size();
            callUpdate();
            bool prevStep = (before == 0) ? false : testIO.commands[before - 1].step;
            for (size_t j = before; j < testIO.commands.size(); ++j)
            {
                bool step = testIO.commands[j].step;
                if (prevStep && !step)
                    if (++seen == n)
                        return *this;
                prevStep = step;
            }
        }
        fail("did not receive %d pulses (saw %d)", n, seen);
        return *this;
    }

    Scenario& hasStepPulses(int expected)
    {
        int actual = countPulses();
        if (expected != actual)
            fail("expected %d pulses, but got %d", expected, actual);
        return *this;
    }

    Scenario& hasForwardStepPulses(int expected)
    {
        int actual = countPulses(true);
        if (expected != actual)
            fail("expected %d forward pulses, but got %d", expected, actual);
        return *this;
    }

    Scenario& hasReverseStepPulses(int expected)
    {
        int actual = countPulses(false);
        if (expected != actual)
            fail("expected %d reverse pulses, but got %d", expected, actual);
        return *this;
    }

    Scenario& hasJointFeedback(int expected)
    {
        int actual = stepgen->getRawCount();
        if (expected != actual)
            fail("expected jointFeedback of %d, but got %d", expected, actual);
        return *this;
    }

    Scenario& madePulsesOfLength(uint32_t length)
    {
        int32_t n = 0;
        for (auto const& c : stepgen->io().recordedStateChanges())
        {
            if (!c.step) continue;
            if (c.cycles != length)
            {
                fail("pulse %d had length %u (expected length %u).", n, c.cycles, length);
                return *this;
            }
            ++n;
        }
        return *this;
    }

    Scenario& outputsStepAndDir(std::vector<TestIO::StateChange> expected)
    {
        std::vector<TestIO::StateChange> actual = stepgen->io().recordedStateChanges();
        if (actual.size() != expected.size())
        {
            fail("produced wrong number of state changes (%u instead of %u)", actual.size(), expected.size());
            return *this;
        }
        for (size_t i = 0; i < expected.size(); ++i)
        {
            auto const& a = actual[i];
            auto const& e = expected[i];
            if (a.cycles != e.cycles || a.step != e.step || a.dir != e.dir)
            {
                fail(
                    "state change %u mismatch: got step=%d (%d), dir=%d (%d), cycles=%u (%u)",
                    i,
                    a.step, e.step,
                    a.dir, e.dir,
                    a.cycles, e.cycles
                );
                return *this;
            }
        }
        return *this;
    }
};

// ---

TEST(test_advancing_cyclesUntilTrigger_always_triggers)
{
    using DDS = BasicDDSAccumulator<75000>;
    DDS dds;

    const int32_t frequencies[] = {25, -25, 1, -1, 80000, -80000};

    for (auto const f : frequencies)
    for (int32_t value = DDS::low; value <= DDS::high; ++value)
    {
        dds.value = value;
        assert(!dds.triggered());
        int32_t cycles = dds.cyclesUntilTrigger(f);
        dds.advance(f, cycles);
        assert(dds.triggered());
    }
}

TEST(test_advancing_less_than_cyclesUntilTrigger_never_triggers)
{
    using DDS = BasicDDSAccumulator<75000>;
    DDS dds;

    const int32_t frequencies[] = {25, -25, 1, -1, 80000, -80000};

    for (auto const f : frequencies)
    for (int32_t value = DDS::low; value <= DDS::high; ++value)
    {
        dds.value = value;
        assert(!dds.triggered());
        int32_t cycles = dds.cyclesUntilTrigger(f);
        dds.advance(f, cycles-1);
        assert(!dds.triggered());
        dds.advance(f, 1);
        assert(dds.triggered());
    }
}

// ---

TEST(test_disabled_joint_does_not_step)
{
    Scenario()
        .withFrequency(100, false)
        .afterRunning1Second()
        .hasStepPulses(0)
        ;
}

TEST(test_zero_frequency_does_not_step)
{
    Scenario()
        .withFrequency(0, true)
        .afterRunning1Second()
        .hasStepPulses(0)
        ;
}

TEST(test_half_rate_steps_every_two_updates)
{
    Scenario()
        .withFrequency(THREAD_FREQ / 2, true)
        .afterRunning1Second()
        .hasStepPulses(THREAD_FREQ / 2)
        ;
}

TEST(test_can_step_once_per_tick)
{
    Scenario()
        .withSteplen(250)
        .withStepspace(250)
        .withFrequency(-THREAD_FREQ)
        .afterRunning1Second()
        .hasStepPulses(THREAD_FREQ)
        .madePulsesOfLength(ceil(CPU_FREQ / 1000000000.0 * 250.0)) 
        ;
}

TEST(test_forward_direction_and_count)
{
    Scenario()
        .withFrequency(THREAD_FREQ / 2, true)
        .afterRunning1Second()
        .hasForwardStepPulses(THREAD_FREQ / 2)
        .hasJointFeedback(THREAD_FREQ / 2)
        ;
}

TEST(test_reverse_direction_and_count)
{
    Scenario()
        .withFrequency(-THREAD_FREQ / 2, true)
        .afterRunning1Second()
        .hasReverseStepPulses(THREAD_FREQ / 2)
        .hasJointFeedback(-THREAD_FREQ / 2)
        ;
}

TEST(test_steplen_greater_than_thread_frequency_keeps_pulse_high_for_multiple_ticks)
{
    Scenario()
        .withSteplen(50000)
        .withFrequency(25)
        .afterPulses(1)
        .outputsStepAndDir({
            { false, false,        0 },
            { false,  true,  25_Hz/2 + 1 }, //???
            {  true,  true, 50000_ns },
            { false,  true,        0 },
        })
        .madePulsesOfLength(2 * CYCLES_PER_TICK)
        ;
}

TEST(test_waits_dirsetup_before_pulsing)
{
    Scenario()
        .withSteplen(50000)
        .withStepspace(50000)
        .withDirsetup(150000)
        .withFrequency(10000)
        .afterPulses(1)
        .outputsStepAndDir({
            { false, false,         0 },
            { false,  true, 150000_ns },
            {  true,  true,  50000_ns },
            { false,  true,         0 },
        });
}

TEST(test_waits_dirhold_before_changing_direction)
{
    Scenario()
        .withSteplen(50000)
        .withStepspace(50000)
        .withDirsetup(75000)
        .withDirhold(150000)
        .withFrequency(-10000)
        .afterPulses(1)
        .withFrequency(THREAD_FREQ/4)
        .afterPulses(1)
        .outputsStepAndDir({
            { false, false,  10_KHz/2 },
            {  true, false,  50000_ns },
            { false, false, 150000_ns },
            { false,  true,  75000_ns },
            {  true,  true,  50000_ns },
            { false,  true,         0 },
        });
}

TEST(test_waits_dirdelay_before_emitting_a_pulse_in_the_opposite_direction)
{
    Scenario()
        .withSteplen(50000)
        .withStepspace(50000)
        .withDirdelay(150000)
        .withFrequency(-THREAD_FREQ/4)
        .afterPulses(1)
        .withFrequency(THREAD_FREQ/4)
        .afterPulses(1)
        .outputsStepAndDir({
            { false, false,                      10_KHz/2 },
            {  true, false,                      50000_ns },
            { false, false,                        1_tick },
            { false,  true, 150000_ns - 50000_ns - 1_tick },
            {  true,  true,                      50000_ns },
            { false,  true,                             0 },
        });
}

int main()
{
    test_advancing_cyclesUntilTrigger_always_triggers();
    test_advancing_less_than_cyclesUntilTrigger_never_triggers();
    test_disabled_joint_does_not_step();
    test_zero_frequency_does_not_step();
    test_half_rate_steps_every_two_updates();
    test_can_step_once_per_tick();
    test_forward_direction_and_count();
    test_reverse_direction_and_count();
    test_steplen_greater_than_thread_frequency_keeps_pulse_high_for_multiple_ticks();
    test_waits_dirsetup_before_pulsing();
    test_waits_dirhold_before_changing_direction();
    test_waits_dirdelay_before_emitting_a_pulse_in_the_opposite_direction();
    if (0 == failures)
        printf("\nAll tests passed.\n");
    exit(failures ? EXIT_FAILURE : EXIT_SUCCESS);
}
