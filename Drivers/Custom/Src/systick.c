#include <stdio.h>
#include <stdint.h>
#include "../Inc/systick.h"

void init_systick_timer(void) {
	/*
	 * 	Initializes the systic timer:
	 * 		First we load the count value onto the SRVR reg. This value is then automatcally copied into SCVR and downcount begins (dont touch)
	 * 		Next, we configure the SCSR register to enable the systick peripheral
	 */
	uint32_t *pSRVR = (uint32_t*) 0xE000E014;
	uint32_t *pSCSR = (uint32_t*) 0xE000E010;
	uint32_t count_value = (SYSTICK_TIM_CLK / TICK_HZ) - 1; // --> 15999 = 14 bits

	*pSRVR &= ~(0x00FFFFFF); // clearing first 24 bits
	*pSRVR |= count_value; // loading count value

	*pSCSR |= (1 << 2); // setting clock source as processor clock
	*pSCSR |= (1 << 1); // enabling invoking of systick exception when count reaches 0
	*pSCSR |= (1 << 0); // enabling the downcounter
}
