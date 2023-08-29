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

extern RxPingPongBuffer rxPingPongBuffer;
extern TxPingPongBuffer txPingPongBuffer;

#endif
