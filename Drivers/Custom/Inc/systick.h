#ifndef SYSTICK_SYSTICK_H_
#define SYSTICK_SYSTICK_H_

#include <stdint.h>

/*
	PUBLIC FUNCTION PROTOTYPES
*/
void init_systick_timer(void);


/*
	OTHER CONSTANTS
*/
#define TICK_HZ 1000U
#define HSI_CLOCK 16000000U
#define SYSTICK_TIM_CLK HSI_CLOCK

#endif
