#include "digitalPin.h"
//#include "../boardconfig.h"


// Only ever 1!

/***********************************************************************
                MODULE CONFIGURATION AND CREATION FROM JSON     
************************************************************************/

void createDigitalPin()
{
    const char* comment = module["Comment"];
    printf("\n%s\n",comment);

    const char* pin = module["Pin"];
    const char* mode = module["Mode"];
    const char* invert = module["Invert"];
    const char* modifier = module["Modifier"];
    int dataBit = module["Data Bit"];

    int mod;
    bool inv;

    if (!strcmp(invert,"True"))
    {
        inv = true;
    }
    else inv = false;

    //ptrOutputs = &pruRxData->outputs;
    //ptrInputs = &pruTxData->inputs;

    printf("Make Digital %s at pin %s\n", mode, pin);

 
    int mode_code = !strcmp(mode, "Output") ? 1 : !strcmp(mode, "Input") ? 0 : -1;
    if(mode_code != -1) {
        DigitalPin::singleton().addPin(mode_code, pin, dataBit, inv, mod);
    } else {
        printf("Invalid pin mode %s, expect 'Input' or 'Output'\n", mode);
    }

}


/***********************************************************************
    MODULE CONFIGURATION AND CREATION FROM STATIC CONFIG - boardconfi.h   
************************************************************************/

void loadStaticIO()
{

}


/***********************************************************************
                METHOD DEFINITIONS
************************************************************************/

void DigitalPin::addPin(int mode, std::string portAndPin, int bitNumber, bool invert, int modifier)
{
    assert(npins < MAXPINS);
    Item& pin = items[npins];
    pin.mode = mode;
    pin.bitNumber = bitNumber;
    pin.invert = invert;
    pin.modifier = modifier;
    pin.pin = new Pin(portAndPin, mode, modifier);
    pin.mask = 1 << bitNumber;
    npins++;
}


void DigitalPin::update()
{
    rxData_t* currentRxPacket = getCurrentRxBuffer(&rxPingPongBuffer);
    txData_t* currentTxPacket = getCurrentTxBuffer(&txPingPongBuffer);
    
    for(int i = 0; i < npins; i++) {
        const Item& item = items[i];
        if(item.mode == 0) {
            bool pinState;
            pinState = item.pin->get();
            if(item.invert) pinState = !pinState;
            if(pinState) {
                currentTxPacket->inputs |= item.mask;
            } else {
                currentTxPacket->inputs &= ~item.mask; 
            }
        } else {
            bool pinState = (currentRxPacket->outputs & item.mask) != 0;
            if(item.invert) pinState = !pinState;
            item.pin->set(pinState);
        }
    }
}

void DigitalPin::slowUpdate()
{

}

DigitalPin& DigitalPin::singleton() {
    static DigitalPin* pin_module = nullptr;
    if(!pin_module) {
        pin_module = new DigitalPin();
        // Not used.
        // servoThread->registerModule(pin_module);
    }
    return *pin_module;
}