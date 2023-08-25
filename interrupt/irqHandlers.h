#include "interrupt.h"
//#include "../hardware/timer.h"

extern "C" {

	void PWM_Wrap_Handler()
	{
		int irq = pwm_get_irq_status_mask();

		if (irq & (1 << 0))
		{
			Interrupt::SLICE0_Wrapper();
			pwm_clear_irq(0);
		}
		else if (irq & (1 << 1))
		{
			Interrupt::SLICE1_Wrapper();
			pwm_clear_irq(1);
		}
	}

	void PWM_Wrap_Handler0()
	{
		hw_clear_bits(&timer_hw->intr, 1u << 0);
		timer_hw->alarm[0] = timer_hw->timerawl + BASE_PERIOD;
		Interrupt::SLICE0_Wrapper();		
	}	
	void PWM_Wrap_Handler1()
	{
		hw_clear_bits(&timer_hw->intr, 1u << 1);
		timer_hw->alarm[1] = timer_hw->timerawl + SERVO_PERIOD;
		Interrupt::SLICE1_Wrapper();
	}		
}
