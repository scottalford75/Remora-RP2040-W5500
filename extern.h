#ifndef EXTERN_H
#define EXTERN_H

#include "configuration.h"
#include "remora.h"

#include "lib/ArduinoJson6/ArduinoJson.h"
#include "thread/pruThread.h"


extern uint32_t base_freq;
extern uint32_t servo_freq;

extern JsonObject module;

// pointers to objects with global scope
extern pruThread* baseThread;
extern pruThread* servoThread;

// unions for RX and TX data pointers that are used by the PRU threads
extern volatile rxData_t* pruRxData;
extern volatile txData_t* pruTxData;

// pointers to data
extern volatile int32_t*   ptrTxHeader;  
extern volatile bool*      ptrPRUreset;
extern volatile int32_t*   ptrJointFreqCmd[JOINTS];
extern volatile int32_t*   ptrJointFeedback[JOINTS];
extern volatile uint8_t*   ptrJointEnable;
extern volatile float*     ptrSetPoint[VARIABLES];
extern volatile float*     ptrProcessVariable[VARIABLES];
extern volatile uint32_t*  ptrInputs;
extern volatile uint32_t*  ptrOutputs;

#endif
