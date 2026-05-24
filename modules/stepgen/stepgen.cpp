#include "stepgen.h"
#include "stepgen.pio.h"

/***********************************************************************
                                    PIOIO
************************************************************************/

PIO PIOIO::lastPio = nullptr;
uint PIOIO::lastOffset = 0;

static int parsePin(std::string const& name)
{
    if (name.length() != 4) return -1;
    if (name[0] != 'G' || name[1] != 'P') return -1;
    return 10 * (name[2] - '0') + (name[3] - '0');
}

bool PIOIO::findStateMachine()
{
    // Prefer reusing the PIO block we already loaded the program into.
    if (nullptr != lastPio)
    {
        int claimed = pio_claim_unused_sm(lastPio, false);
        if (claimed >= 0)
        {
            pio = lastPio;
            sm = (uint)claimed;
            offset = lastOffset;
            printf("PIOIO: reusing program on PIO%d sm=%u\n", PIO_NUM(pio), sm);
            return true;
        }
    }

    if (!pio_claim_free_sm_and_add_program(&stepgen_program, &pio, &sm, &offset))
    {
        printf("PIOIO: could not claim a state machine\n");
        return false;
    }

    lastPio = pio;
    lastOffset = offset;
    return true;
}

PIOIO::PIOIO(std::string const& step, std::string const& direction)
    : stepPin(parsePin(step))
    , dirPin(parsePin(direction))
    , pio(nullptr)
    , sm(0)
    , offset(0)
{
    if (stepPin < 0 || dirPin < 0)
    {
        printf("PIOIO: bad pin name(s) step='%s' dir='%s'\n", step.c_str(), direction.c_str());
        return;
    }

    if (!findStateMachine())
        return;

    pio_gpio_init(pio, stepPin);
    pio_sm_set_consecutive_pindirs(pio, sm, stepPin, 1, true);
    pio_gpio_init(pio, dirPin);
    pio_sm_set_consecutive_pindirs(pio, sm, dirPin, 1, true);

    pio_sm_config config = stepgen_program_get_default_config(offset);
    sm_config_set_sideset_pins(&config, stepPin);     // step driven by side-set
    sm_config_set_out_pins(&config, dirPin, 1);       // dir driven by OUT pins
    sm_config_set_fifo_join(&config, PIO_FIFO_JOIN_TX); // RX unused; double TX depth
    pio_sm_init(pio, sm, offset, &config);
    pio_sm_set_enabled(pio, sm, true);

    printf("PIOIO: configured step=GP%02d dir=GP%02d on PIO%d sm=%u offset=%u\n",
           stepPin, dirPin, PIO_NUM(pio), sm, offset);
}

/***********************************************************************
                MODULE CONFIGURATION AND CREATION FROM JSON
************************************************************************/

Stepgen *createStepgen()
{
    const char* comment = module["Comment"];
    printf("\n%s\n",comment);

    const char* step = module["Step Pin"];
    const char* dir = module["Direction Pin"];

    float steplen = module["steplen"];
    float stepspace = module["stepspace"];
    float dirsetup = module["dirsetup"];
    float dirhold = module["dirhold"];
    float dirdelay = module["dirdelay"];

    // create the step generator, register it in the thread
    Stepgen* stepgen = new Stepgen(
        step, dir, steplen, stepspace,
        dirsetup, dirhold, dirdelay
    );
    BaseThread::registerModule(stepgen);
    return stepgen;
}
