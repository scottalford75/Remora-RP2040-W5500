#ifndef STEPGEN_H
#define STEPGEN_H

#include <cstdint>

#include "hardware/pio.h"

#include "../extern.h"
#include "../../configuration.h"
#include "../../drivers/pin/pin.h"

#include "basic_stepgen.h"

class PIOIO
{
private:
    int stepPin;
    int dirPin;
    PIO pio;
    uint sm;
    uint offset;

    // Reuse a PIO block we already loaded the program into when possible,
    // so multiple stepgens can share program memory.
    static PIO lastPio;
    static uint lastOffset;

    bool findStateMachine();

public:
    PIOIO(std::string const& step, std::string const& direction);

    // PIO program overhead per FIFO entry, in PIO cycles.
    // See stepgen.pio for the derivation.
    static constexpr uint32_t programOverhead = 6;

    inline uint32_t schedule(uint32_t cycles, bool step, bool dir)
    {
        uint32_t wait = cycles > programOverhead ? cycles - programOverhead : 0;
        uint32_t cmd = (wait & 0x3FFFFFFFu)
                     | ((uint32_t)step << 30)
                     | ((uint32_t)dir  << 31);
        pio_sm_put_blocking(pio, sm, cmd);
        return wait + programOverhead;
    }
};

using Stepgen = BasicStepgen<PLL_SYS_KHZ * 1000, PRU_BASEFREQ, PIOIO>;

Stepgen *createStepgen(void);

#endif
