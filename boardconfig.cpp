#include "boardconfig.h"

// Note: Update the number of stepgens "NUM_STEPGEN" etc in configuration.h to match the configs in this file


// -------- Servo thread modules --------

// Blink is good for testing as we can use the onboard LED on GP25
BlinkPinConfig BlinkConfigs[NUM_BLINK] = {{"Blinky on GP25", "GP25", 2}};    



// -------- Base thread modules --------
                                    
StepgenConfig StepgenConfigs[NUM_STEPGEN] = {{"X-Axis", 0, "GP02", "GP03"}, 
                                             {"Y-Axis", 1, "GP04", "GP05"},
                                             {"Z-Axis", 2, "GP06", "GP07"},
                                             {"A-Axis", 3, "GP08", "GP09"}};





