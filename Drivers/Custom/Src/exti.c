#include "stdint.h"
#include "stdio.h"
#include "../Inc/exti.h"
#include "../../../Core/Inc/registers.h"

void exti_init(void) {
	// Enable clock to SYSCFG peripheral (hangs off APB2 - 14th bit)
	uint32_t *pRCC_APB2ENR = (uint32_t*) (RCC_BASE_ADDRESS + 0x44);
	*pRCC_APB2ENR |= (1 << 14);

	// Enable clock to GPIOA peripheral (hangs off AHB1 - 0th bit)
	uint32_t *pRCC_AHB1ENR = (uint32_t*) (RCC_BASE_ADDRESS + 0x30);
	*pRCC_AHB1ENR |= (1 << 0);

	// Configure SYSCFG_EXTICR1 for PA0 & PA1 (setting EXTI0[0:3] and EXTI1[4:7] to 0)
	uint32_t *pSYSCFG_EXTICR1 = (uint32_t*) (SYSCFG_BASE_ADDRESS + 0x08);
	*pSYSCFG_EXTICR1 &= ~(0xFF);

	// Configure PA0 & PA1 pins to INPUT mode
	uint32_t *pGPIOA_MODER = (uint32_t*) (GPIOA_BASE_ADDRESS + 0x00);
	*pGPIOA_MODER &= ~(0xF);

	// Writing EXTI_PR to 0 (write 1 to clear semantic so we write 1 to clear the register)
	// we write = instead of set |=, because PR is event-driven
	uint32_t *pEXTI_PR = (uint32_t*) (EXTI_BASE_ADDRESS + 0x14);
	*pEXTI_PR = (0x3);

	// Configuring clock trigger mechanism to rising edge trigger:
	// 				2. Unmasking the lines in EXTI_IMR (pending event is allowed through)
	//				3. Selecting trigger edge to rising --> RTSR
	uint32_t *pEXTI_IMR = (uint32_t*) (EXTI_BASE_ADDRESS + 0x00);
	*pEXTI_IMR |= 0x3;
	uint32_t *pEXTI_RTSR = (uint32_t*) (EXTI_BASE_ADDRESS + 0x08);
	*pEXTI_RTSR |= 0x3;
}
