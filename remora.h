#ifndef REMORA_H
#define REMORA_H
#pragma pack(push, 1)


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

//extern volatile rxData_t rxData1;
//extern volatile rxData_t rxData2;

//extern volatile rxData_t rxData_buf[2];


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

//extern volatile txData_t txData1;
//extern volatile rxData_t txData2;

//extern volatile txData_t txData_buf[2];

// pointers to data
//extern volatile txData_t* prutxData;
//extern volatile rxData_t* pruRxData;


#pragma pack(pop)
#endif