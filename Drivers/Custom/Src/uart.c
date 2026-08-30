#include <stdint.h>
#include <stdio.h>
#include "../Inc/uart.h"
#include "../../../Core/Inc/registers.h"

volatile uint32_t rx_byte_count;

void init_uart(void) {
	/*
	 *	This function enables the USART2 peripheral configured for UART transmission
	 */

	// Enable clock to APB1 and GPIOA peripherals
	uint32_t *pRCC_APB1ENR = (uint32_t*) (RCC_BASE_ADDRESS + 0x40);
	uint32_t *pRCC_AHB1ENR = (uint32_t*) (RCC_BASE_ADDRESS + 0x30);
	*pRCC_APB1ENR |= (1 << 17);
	*pRCC_AHB1ENR |= (1 << 0);

	// Setting PA2 and PA3 to alternate function mode
	uint32_t *pGPIOA_MODER = (uint32_t*) (GPIOA_BASE_ADDRESS + 0x00);
	*pGPIOA_MODER &= ~((3 << 4) | (3 << 6)); // clear
	*pGPIOA_MODER |= (2 << 4) | (2 << 6); // set

	// Setting AF7 to TX for PA2 and RX for PA3 --> configure the AFR register:
	// 			AFRL --> covers pins 0 to 7
	// 			AFRH --> covers pins 8 to 15
	// PA2 and PA3 are both 0-7 --> AFRL
	uint32_t *pGPIOA_AFRL = (uint32_t*) (GPIOA_BASE_ADDRESS + 0x20);
	*pGPIOA_AFRL &= ~((15 << 8) | (15 << 12)); // clear
	*pGPIOA_AFRL |= (7 << 8) | (7 << 12); // set

	// Loading BRR with f_CK / baud rate
	uint32_t *pUSART_BRR = (uint32_t*) (USART2_BASE_ADDRESS + 0x08);
	*pUSART_BRR = BRR_VALUE;

	// Configuring CR1 to enable USART, transmission,  receiving and RXNEIE for triggering interrupts for RX
	uint32_t *pUSART_CR1 = (uint32_t*) (USART2_BASE_ADDRESS + 0x0C);
	*pUSART_CR1 |= ((1 << 13) | (1 << 3) | (1 << 2) | (1 << 5));

	// Enabling interrupt at NVIC level for USART2 (IRQ 38)
	uint32_t *pNVIC_ISER1 = (uint32_t*) (NVIC_BASE_ADDRESS + 0x4);
	*pNVIC_ISER1 |= (1 << 6);

	// Setting priority level of USART2 to 1
	uint32_t *pNVIC_IPR9 = (uint32_t*) (NVIC_IPR_BASE_ADDRESS + (0x4 * 9));
	*pNVIC_IPR9 &= ~(0xFUL << 20); // clear
	*pNVIC_IPR9 |= (0x1UL << 20); // set
}

void USART2_IRQHandler(void) {
	uint32_t RXNE_flag;
	uint32_t data;

	uint32_t *pUSART_SR = (uint32_t*) (USART2_BASE_ADDRESS + 0x00);
	RXNE_flag = (*pUSART_SR >> 5) & 0x01; // reads RXNE value (5th bit)

	uint32_t *pUSART_DR = (uint32_t*) (USART2_BASE_ADDRESS + 0x04);
	data = (*pUSART_DR & 0xFF); // reads data value [8:0]

	rx_byte_count++;
}

void uart_write_char(char c) {
	/*
	 * 1. Wait until TXE flag is set in SR
	 * 2. Once set, write bit to DR
	 * 3. Hardware handles the rest
	 *
	 */
	uint32_t *pUSART_SR = (uint32_t*) (USART2_BASE_ADDRESS + 0x00);
	while (!((*pUSART_SR >> 7) & 1)); // keep waiting until TXE flag is set.
	// ((*pUSART_SR >> 7) & 1 gets value of bit 7) i.e TXE
	uint32_t *pUSART_DR = (uint32_t*) (USART2_BASE_ADDRESS + 0x04);
	*pUSART_DR = c;
}

void uart_write_string(char str[]) {
	char *ptr = str; // ptr points to str[0]
	while (*ptr != '\0') { // *ptr --> dereferencing and getting actual value
		uart_write_char(*ptr);
		ptr++;
	}
}

void uart_write_int(uint32_t value) {
	char buf[12];
	sprintf(buf, "%lu", value);
	uart_write_string(buf);
}

void uart_write_hex(uint32_t value) {
	char buf[12];
	sprintf(buf, "0x%08lX", value);
	uart_write_string(buf);
}


