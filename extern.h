#ifndef EXTERN_H
#define EXTERN_H

#include "configuration.h"
#include "remora.h"

#include "lib/ArduinoJson6/ArduinoJson.h"
#include "thread/pruThread.h"


extern JsonObject module;

// unions for RX and TX data pointers that are used by the PRU threads

extern RxPingPongBuffer rxPingPongBuffer;
extern TxPingPongBuffer txPingPongBuffer;

#endif
