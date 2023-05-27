#include "hardware/irq.h"
#include "hardware/pwm.h"

#include <stdio.h>

#include "../configuration.h"
#include "../interrupt/interrupt.h"
#include "timerInterrupt.h"
#include "timer.h"
#include "pruThread.h"

extern "C" void PWM_Wrap_Handler();

// Timer constructor
pruTimer::pruTimer(uint8_t slice, uint32_t frequency, pruThread* ownerPtr):
	slice(slice),
	frequency(frequency),
	timerOwnerPtr(ownerPtr)
{
	interruptPtr = new TimerInterrupt(this->slice, this);	// Instantiate a new Timer Interrupt object and pass "this" pointer

	this->startTimer();
}


void pruTimer::timerTick(void)
{
	//Do something here
	this->timerOwnerPtr->run();
}



void pruTimer::startTimer(void)
{
    uint32_t wrap, div;

    printf("    setting up timer on PWM Slice %d\n", this->slice);

    // We want exact frequencies for the threads so a combination of divider and top needs to be found
    // 40khz 	Top = 3125-1	    Div = 1.0    <-- Base thread
    // 1khz		Top = 62500-1	    Div = 2.0    <-- Servo thread

    // Find a combination of wrap and div for the desired frequency - this may not give the exact frequency
    // TODO - maybe look for more accurate combinations?
   
    wrap = (PLL_SYS_KHZ * 1000) / this->frequency;
    div = 1;
    //printf("    frequcney = %d, wrap = %d\n", this->frequency, wrap);
    while (wrap > 65535)
    {
        wrap >>= 1;
        div <<= 1;
        //printf("    wrap = %d, div =%d\n", wrap, div);
    }
    if(div > 256) printf("    Invalid frequency\n");
    wrap--;

    printf("    desired frequency = %d, div = %d, wrap = %d\n", frequency, div, wrap);
    printf("    actual frequency  = %6.4f\n", float((PLL_SYS_KHZ * 1000)/ div / (wrap + 1)));

    pwm_set_clkdiv(this->slice, div);
    pwm_set_wrap(this->slice, wrap);
    pwm_set_irq_enabled(this->slice, true);
    pwm_set_enabled(this->slice, true);

    irq_set_exclusive_handler(PWM_IRQ_WRAP, PWM_Wrap_Handler);
    irq_set_priority(PWM_IRQ_WRAP, 0);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    printf("	timer started\n");
}

void pruTimer::stopTimer()
{
    printf("	timer stop\n\r");
    irq_set_enabled(PWM_IRQ_WRAP, false);
}