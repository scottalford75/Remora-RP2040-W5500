#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "../configuration.h"

#define BASE_PERIOD 1000000 / PRU_BASEFREQ
#define SERVO_PERIOD 1000000 / PRU_SERVOFREQ

// Base class for all interrupt derived classes

#define PERIPH_COUNT_IRQn	8				// Total number of device interrupt sources - 8 PWM Slices (for the moment)



class Interrupt
{
	protected:

		static Interrupt* ISRVectorTable[PERIPH_COUNT_IRQn];

	public:

		Interrupt(void);

		static void Register(int interruptNumber, Interrupt* intThisPtr);

		// wrapper functions to ISR_Handler()
		static void SLICE0_Wrapper();
        static void SLICE1_Wrapper();

		virtual void ISR_Handler(void) = 0;

};

#endif
