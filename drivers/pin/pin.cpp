#include "pin.h"
#include <cstdio>
#include <cerrno>
#include <string>


Pin::Pin(std::string portAndPin, int dir) :
    portAndPin(portAndPin),
    dir(dir)
{
    printf("Creating Pin @\n");

    if (this->portAndPin[0] == 'G') // GPXX e.g.GP01 GP21
    {  
        this->pinNumber     = this->portAndPin[3] - '0';       
        uint16_t pin2       = this->portAndPin[2] - '0';       

        if (pin2 > 0) 
        {
            this->pinNumber = pin2 * 10 + this->pinNumber ;
        }

        this->pin = this->pinNumber;
    }
    else
    {
        printf("  Invalid port and pin definition\n");
        return;
    }    

    printf("  pin = GP%02d\n", this->pinNumber);

    gpio_init(this->pin);

    if (this->dir == INPUT)
    {
        gpio_set_dir(this->pin, GPIO_IN);
    }
    else if (this->dir == OUTPUT)
    {
        gpio_set_dir(this->pin, GPIO_OUT);
    }

}
