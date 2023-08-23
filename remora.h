#ifndef REMORA_H
#define REMORA_H
#pragma pack(push, 1)

#include "configuration.h"

typedef union
{
  // this allow structured access to the incoming SPI data without having to move it
  struct
  {
    uint8_t rxBuffer[BUFFER_SIZE];
  };
  struct
  {
    int32_t header;
    volatile int32_t jointFreqCmd[JOINTS]; 	// Base thread commands ?? - basically motion
    float setPoint[VARIABLES];		  // Servo thread commands ?? - temperature SP, PWM etc
    uint8_t jointEnable;
    volatile uint32_t outputs;
    uint8_t spare0;
  };
} rxData_t;

typedef union
{
  // this allow structured access to the out going SPI data without having to move it
  struct
  {
    uint8_t txBuffer[BUFFER_SIZE];
  };
  struct
  {
    int32_t header;
    int32_t jointFeedback[JOINTS];	  // Base thread feedback ??
    float processVariable[VARIABLES];		     // Servo thread feedback ??
	  uint32_t inputs;
	  uint16_t NVMPGinputs;
  };
} txData_t;

typedef struct {
    rxData_t rxBuffers[2]; // Two buffers for rxData_t
    int currentRxBuffer;   // Index of the current rxData_t buffer
} RxPingPongBuffer;

typedef struct {
    txData_t txBuffers[2]; // Two buffers for txData_t
    int currentTxBuffer;   // Index of the current txData_t buffer
} TxPingPongBuffer;

extern void initRxPingPongBuffer(RxPingPongBuffer* buffer);
extern void initTxPingPongBuffer(TxPingPongBuffer* buffer);
extern void swapRxBuffers(RxPingPongBuffer* buffer);
extern void swapTxBuffers(TxPingPongBuffer* buffer);
extern rxData_t* getCurrentRxBuffer(RxPingPongBuffer* buffer);
extern txData_t* getCurrentTxBuffer(TxPingPongBuffer* buffer);

#pragma pack(pop)
#endif