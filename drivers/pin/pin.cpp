#include "pin.h"
#include <cstdio>
#include <cerrno>
#include <string>


Pin::Pin(std::string portAndPin, int dir) :
    portAndPin(portAndPin),
    dir(dir)
{
    // Set direction
    if (this->dir == INPUT)
    {
        this->mode = GPIO_IN;
        this->pullup = false;
        this->pulldown = false;
    }
    else
    {
        this->mode = GPIO_OUT;
        this->pullup = false;
        this->pulldown = false;
    }

    this->configPin();
}


Pin::Pin(std::string portAndPin, int dir, int modifier) :
    portAndPin(portAndPin),
    dir(dir),
    modifier(modifier)
{
    // Set direction
    if (this->dir == INPUT)
    {
        this->mode = GPIO_IN;

        // Set pin  modifier
        switch(this->modifier)
        {
            case PULLUP:
                printf("  Setting pin as Pull Up\n");
                this->pullup = true;
                this->pulldown = false;
                break;
            case PULLDOWN:
                printf("  Setting pin as Pull Down\n");
                this->pullup = false;
                this->pulldown = true;
                break;
            case PULLNONE:
                printf("  Setting pin as No Pull\n");
                this->pullup = false;
                this->pulldown = false;
                break;
        }
    }
    else
    {
        this->mode = GPIO_OUT;
        this->pullup = false;
        this->pulldown = false;
    }

    this->configPin();
}

void Pin::configPin()
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
    gpio_set_dir(this->pin, this->mode);
    gpio_set_pulls(this->pin, this->pullup, this->pulldown);
}