#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include "../../remora.h"


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

// --- Pin stub ---

std::map<std::string, bool> pinState;

struct TestPin
{
    std::string name;
    TestPin(std::string name, int) : name(name) {}
    void set(bool v) { pinState[name] = v; }
    bool get() const { return pinState.count(name) ? pinState.at(name) : false; }
};


#define OUTPUT 42

#include "../../modules/stepgen/basic_stepgen.h"

// --- Buffer getter stubs (declared extern in remora.h) ---

using TestStepgen = BasicStepgen<TestPin>;

static const int32_t THREAD_FREQ = 40000;
static const char* STEP_PIN = "GP02";
static const char* DIR_PIN  = "GP03";

struct Sample
{
    bool step;
    bool dir;
    int  count;
};

using Step = std::vector<int>;
using Dir = std::vector<int>;
using Count = std::vector<int>;

class Scenario
{
private:
    std::vector<Sample> samples;
    int32_t threadFreq;
    int32_t steplen;
    int32_t stepspace;
    int32_t dirsetup;
    int32_t dirhold;
    int32_t dirdelay;
    std::optional<TestStepgen> stepgen;

    std::pair<int, int> countPulses()
    {
        std::pair<int, int> result;
        bool last = false;
        for (auto const& sample : samples)
        {
            if (!last && sample.step)
            {
                if (sample.dir)
                    ++result.first;
                else
                    ++result.second;
            }
            last = sample.step;
        }
        return result;
    }

    void start()
    {
        if (!stepgen.has_value())
        {
            stepgen.emplace(threadFreq, 0, STEP_PIN, DIR_PIN, steplen, stepspace, dirsetup, dirhold, dirdelay);
            samples.push_back(Sample{pinState[STEP_PIN], pinState[DIR_PIN], 1});
        }
    }

    void sample()
    {
        if (samples.back().step == pinState[STEP_PIN] && samples.back().dir == pinState[DIR_PIN])
            ++samples.back().count;
        else
            samples.push_back(Sample{pinState[STEP_PIN], pinState[DIR_PIN], 1});
    }

    void fail(const char *msg, ...)
    {
        va_list args;
        ++failures;
        printf("\n\n\e[31mFAILED\e[0m: ");
        va_start(args, msg);
        vprintf(msg, args);
        va_end(args);

        dumpSamples();
    }

public:
    Scenario()
      : threadFreq(THREAD_FREQ)
      , steplen(0)
      , stepspace(0)
      , dirsetup(0)
      , dirhold(0)
      , dirdelay(0)
    {
        pinState.clear();
    }

    Scenario& dumpSamples(int n = 45)
    {
        n = std::min(n, int(samples.size()));
        printf("\n Step:");
        for (int i = 0; i < n; i++)
            printf("%d ", samples[i].step);
        printf("\n  Dir:");
        for (int i = 0; i < n; i++)
            printf("%d ", samples[i].dir);
        printf("\nCount:");
        for (int i = 0; i < n; i++)
            printf("%d ", samples[i].count);
        return *this;
    }

    Scenario& withThreadFrequency(int32_t value) { threadFreq = value; return *this; }
    Scenario& withSteplen(int32_t value) { steplen = value; return *this; }
    Scenario& withStepspace(int32_t value) { stepspace = value; return *this; }
    Scenario& withDirsetup(int32_t value) { dirsetup = value; return *this; }
    Scenario& withDirhold(int32_t value) { dirhold = value; return *this; }
    Scenario& withDirdelay(int32_t value) { dirdelay = value; return *this; }
    Scenario& withDirPin(bool b) { pinState[DIR_PIN] = b; return *this; }

    Scenario& withFrequency(int32_t frequency, bool enabled = true)
    {
        start();
        stepgen->setFrequency(frequency, enabled);
        return *this;
    }

    Scenario& afterRunning1Second()
    {
        start();
        for (int i = 0; i < threadFreq; i++)
        {
            stepgen->update();
            sample();
        }
        return *this;
    }

    Scenario& afterPulses(int n)
    {
        start();
        int seen = 0;
        for (int i = 0; i < threadFreq; ++i)
        {
            stepgen->update();
            sample();

            if (samples.size() < 2) continue;
            const auto& a = samples[samples.size()-2];
            const auto& b = samples[samples.size()-1];
            // First sample of falling edge.
            if (a.step && !b.step && b.count == 1)
            {
                if (++seen == n)
                    return *this;
            }
        }
        fail("did not receive %d pulses (saw %d)", n, seen);
        return *this;
    }

    Scenario& hasStepPulses(int expected)
    {
        auto pulses = countPulses();
        int actual = pulses.first + pulses.second;
        if (expected != actual)
            fail("expected %d pulses, but got %d", expected, actual);
        return *this;
    }

    Scenario& hasForwardStepPulses(int expected)
    {
        auto pulses = countPulses();
        int actual = pulses.first;
        if (expected != actual)
            fail("expected %d forward pulses, but got %d", expected, actual);
        return *this;
    }

    Scenario& hasReverseStepPulses(int expected)
    {
        auto pulses = countPulses();
        int actual = pulses.second;
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

    Scenario& madePulsesOfLength(int sampleLength)
    {
        int32_t n = 0;
        int32_t length = 0;
        for (auto const& sample : samples)
        {
            if (sample.step) length += sample.count;
            if (length && !sample.step)
            {
                if (length != sampleLength)
                {
                    fail("pulse %d had length %d (expected length %d).", n, length, sampleLength);
                    return *this;
                }
                length = 0;
            }
        }
        return *this;
    }

    Scenario& producesSamples(Step steps, Dir dirs, Count counts)
    {
        bool equal = true;
        for (int i = 0; i < steps.size(); i++)
            if (samples[i].step != bool(steps[i]) || samples[i].dir != bool(dirs[i]) || samples[i].count != counts[i])
                equal = false;
        if (!equal)
            fail("produced the wrong samples");
        return *this;
    }
};

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
        .withDirPin(true)
        .withFrequency(THREAD_FREQ / 2, true)
        .afterRunning1Second()
        .hasStepPulses(THREAD_FREQ / 2)
        ;
}

TEST(test_forward_direction_and_count)
{
    Scenario()
        .withDirPin(true)
        .withFrequency(THREAD_FREQ / 2, true)
        .afterRunning1Second()
        .hasForwardStepPulses(THREAD_FREQ / 2)
        .hasJointFeedback(THREAD_FREQ / 2)
        ;
}

TEST(test_reverse_direction_and_count)
{
    Scenario()
        .withDirPin(false)
        .withFrequency(-THREAD_FREQ / 2, true)
        .afterRunning1Second()
        .hasReverseStepPulses(THREAD_FREQ / 2)
        .hasJointFeedback(-THREAD_FREQ / 2)
        ;
}

TEST(test_steplen_greater_than_frequency_keeps_pulse_high_for_multiple_ticks)
{
    Scenario()
        .withDirPin(true)
        .withThreadFrequency(40000)
        .withSteplen(50000)
        .withFrequency(25)
        .afterRunning1Second()
        .producesSamples(
             Step{0,    1, 0},
              Dir{1,    1, 1},
            Count{1601, 2, 1598}
        )
        .madePulsesOfLength(2)
        ;
}

TEST(test_clamps_maximum_frequency_to_honor_steplen_and_stepspace)
{
    Scenario()
        .withDirPin(true)
        .withThreadFrequency(40000)
        .withSteplen(50000)
        .withStepspace(50000)
        .withFrequency(THREAD_FREQ, true)
        .afterRunning1Second()
        .hasStepPulses(10000)
        ;
    Scenario()
        .withDirPin(false)
        .withThreadFrequency(40000)
        .withSteplen(50000)
        .withStepspace(50000)
        .withFrequency(-THREAD_FREQ, true)
        .afterRunning1Second()
        .hasStepPulses(10000)
        ;
}

TEST(test_waits_dirsetup_before_pulsing)
{
    Scenario()
        .withDirPin(false)
        .withThreadFrequency(40000)
        .withSteplen(50000)
        .withStepspace(50000)
        .withDirsetup(150000)
        .withFrequency(THREAD_FREQ/2)
        .afterRunning1Second()
        .producesSamples(
             Step{0,0,1,0},
              Dir{0,1,1,1},
            Count{1,6,2,2}
        );
    Scenario()
        .withDirPin(true)
        .withThreadFrequency(40000)
        .withSteplen(50000)
        .withStepspace(50000)
        .withDirsetup(150000)
        .withFrequency(-THREAD_FREQ/2)
        .afterRunning1Second()
        .producesSamples(
             Step{0,0,1,0},
              Dir{1,0,0,0},
            Count{1,6,2,2}
        );
}

TEST(test_waits_dirhold_before_changing_direction)
{
    Scenario()
        .withDirPin(false)
        .withThreadFrequency(40000)
        .withSteplen(50000)
        .withStepspace(50000)
        .withDirsetup(75000)
        .withDirhold(150000)
        .withFrequency(-THREAD_FREQ/2)
        .afterPulses(1)
        .withFrequency(THREAD_FREQ/2)
        .afterRunning1Second()
        .producesSamples(
             Step{0, 1, 0, 0, 1, 0},
              Dir{0, 0, 0, 1, 1, 1},
            Count{1, 2, 6, 3, 2, 2}
        );
}

TEST(test_waits_dirdelay_before_emitting_a_pulse_in_the_opposite_direction)
{
    Scenario()
        .withDirPin(false)
        .withThreadFrequency(40000)
        .withSteplen(50000)
        .withStepspace(50000)
        .withDirdelay(150000)
        .withFrequency(-THREAD_FREQ/2)
        .afterPulses(1)
        .withFrequency(THREAD_FREQ/2)
        .afterPulses(1)
        .producesSamples(
             Step{0, 1, 0, 0, 1, 0},
              Dir{0, 0, 0, 1, 1, 1},
            Count{1, 2, 1, 3, 2, 1}
        );
}

int main()
{
    test_disabled_joint_does_not_step();
    test_zero_frequency_does_not_step();
    test_half_rate_steps_every_two_updates();
    test_forward_direction_and_count();
    test_reverse_direction_and_count();
    test_steplen_greater_than_frequency_keeps_pulse_high_for_multiple_ticks();
    test_clamps_maximum_frequency_to_honor_steplen_and_stepspace();
    test_waits_dirsetup_before_pulsing();
    test_waits_dirhold_before_changing_direction();
    test_waits_dirdelay_before_emitting_a_pulse_in_the_opposite_direction();
    if (0 == failures)
        printf("\nAll tests passed.\n");
    exit(failures ? EXIT_FAILURE : EXIT_SUCCESS);
}
