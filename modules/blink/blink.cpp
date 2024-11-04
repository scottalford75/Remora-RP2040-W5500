#include "blink.h"
//#include "../boardconfig.h"

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
    Blink::singleton().addBlink(pin, servo_freq, freq);
}

/***********************************************************************
    MODULE CONFIGURATION AND CREATION FROM STATIC CONFIG - boardconfi.h   
************************************************************************/

void loadStaticBlink()
{

}

/***********************************************************************
                METHOD DEFINITIONS
************************************************************************/

void Blink::addBlink(std::string portAndPin, uint32_t threadFreq, uint32_t freq)
{
	if(!blinkPin) {
		this->periodCount = threadFreq / freq;
		this->blinkCount = 0;
		this->bState = false;

		this->blinkPin = new Pin(portAndPin, OUTPUT);
		this->blinkPin->set(bState);
	} else {
		printf("Ignoring additional blink pin, already have one.\n");
	}
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


Blink& Blink::singleton() {
	static Blink* blink = nullptr;
	if(!blink) {
		blink = new Blink();
		// Not used
        // servoThread->registerModule(blink);
	}
	return *blink;
}