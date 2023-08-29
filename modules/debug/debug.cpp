#include "debug.h"


Debug::Debug(std::string portAndPin, bool bstate) :
    bState(bstate)
{
	this->debugPin = new Pin(portAndPin, OUTPUT);
}

void Debug::update(void)
{
	static bool value;
	value = !value;
	this->debugPin->set(value);
}

void Debug::slowUpdate(void)
{
	return;
}
