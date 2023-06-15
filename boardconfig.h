#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H

#include "configuration.h"
#include "drivers/pin/pin.h"

struct BlinkPinConfig {
    const char* Comment;
    const char* Pin;
    const int Freq;
};

struct DigitalPinConfig {
    const char* Comment;
    const char* Pin;
    const int Modifier;
    const bool Invert;
    const int DataBit;
};

struct StepgenConfig {
    const char* Comment;
    const int JointNumber;
    const char* StepPin;
    const char* DirectionPin;

}; 


//extern BlinkPinConfig BlinkConfigs[NUM_BLINK];
//extern DigitalPinConfig DIConfigs[NUM_INPUTS];
//extern DigitalPinConfig DOConfigs[NUM_OUTPUTS];
//extern StepgenConfig StepgenConfigs[NUM_STEPGEN];

// -------- Servo thread modules --------

// Blink is good for testing as we can use the onboard LED on GP25
BlinkPinConfig BlinkConfigs[] = {{"Blinky on GP25", "GP25", 2}};  

DigitalPinConfig DIConfigs[] = {{"X_LIMIT", "GP11", PULLDOWN, true, 0},
                                {"Y_LIMIT", "GP12", PULLDOWN, true, 1},
                                {"Z_LIMIT", "GP13", PULLDOWN, true, 2}};  //Comment, pin, modifier, invert, data bit

DigitalPinConfig DOConfigs[] = {{"AUX0", "GP09", PULLNONE, false, 0}, 
                                {"AUX1", "GP10", PULLNONE, false, 1}}; //Comment, pin, modifier, invert, data bit

// -------- Base thread modules --------
                                    
StepgenConfig StepgenConfigs[] = {{"X-Axis", 0, "GP02", "GP03"}, 
                                  {"Y-Axis", 1, "GP04", "GP05"},
                                  {"Z-Axis", 2, "GP06", "GP07"},
                                  {"A-Axis", 3, "GP08", "GP09"}};

#endif