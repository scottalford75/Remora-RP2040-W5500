#include "interrupt.h"

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
}
