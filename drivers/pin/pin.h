#ifndef PIN_H
#define PIN_H

#include "pico/stdlib.h"

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <string>

#define INPUT 0x0
#define OUTPUT 0x1

#define PULLUP      0b010
#define PULLDOWN    0b011
#define PULLNONE    0b100

class Pin
{
    private:

        std::string         portAndPin;
        uint8_t             dir;
        uint8_t             modifier;
        uint16_t            pinNumber;
        uint16_t            pin;
        uint32_t            mode;
        bool                pullup;
        bool                pulldown;

    public:

        Pin(std::string, int);
        Pin(std::string, int, int);

        void configPin();

        inline bool get()
        {
            return gpio_get(this->pin);
        }

        inline void set(bool value)
        {
            if (value)
            {
                gpio_put(this->pin, 1);
            }
            else
            {
                gpio_put(this->pin, 0);
            }
        }
};

#endif
