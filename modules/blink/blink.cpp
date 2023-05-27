#include "blink.h"
#include "../boardconfig.h"

/***********************************************************************
                MODULE CONFIGURATION AND CREATION FROM JSON     
************************************************************************/

void createBlink()
{
    const char* comment = module["Comment"];
    printf("\n%s\n",comment);

    const char* pin = module["Pin"];
    int freq = module["Frequency"];

    // create the blink module
    Module* blink = new Blink(pin, servo_freq, freq);
    servoThread->registerModule(blink);
}

/***********************************************************************
    MODULE CONFIGURATION AND CREATION FROM STATIC CONFIG - boardconfi.h   
************************************************************************/

void loadStaticBlink()
{
	    for (int i = 0; i < sizeof(BlinkConfigs)/sizeof(*BlinkConfigs); i++) {
        printf("\nMake Blink at pin %s\n", BlinkConfigs[i].Comment, BlinkConfigs[i].Pin, BlinkConfigs[i].Freq);
        Module* blink = new Blink(BlinkConfigs[i].Pin, servo_freq, BlinkConfigs[i].Freq);
        servoThread->registerModule(blink);
    }
}

/***********************************************************************
                METHOD DEFINITIONS
************************************************************************/

Blink::Blink(std::string portAndPin, uint32_t threadFreq, uint32_t freq)
{

	this->periodCount = threadFreq / freq;
	this->blinkCount = 0;
	this->bState = false;

	this->blinkPin = new Pin(portAndPin, OUTPUT);
	this->blinkPin->set(bState);
}

void Blink::update(void)
{
	++this->blinkCount;
	if (this->blinkCount >= this->periodCount / 2)
	{
		this->blinkPin->set(this->bState=!this->bState);
		this->blinkCount = 0;
	}
}

void Blink::slowUpdate(void)
{
	return;
}
