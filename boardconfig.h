#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H

#include "configuration.h"

struct BlinkPinConfig {
    const char* Comment;
    const char* Pin;
    const int Freq;
};

struct StepgenConfig {
    const char* Comment;
    const int JointNumber;
    const char* StepPin;
    const char* DirectionPin;

}; 

extern StepgenConfig StepgenConfigs[NUM_STEPGEN];
extern BlinkPinConfig BlinkConfigs[NUM_BLINK];

#endif