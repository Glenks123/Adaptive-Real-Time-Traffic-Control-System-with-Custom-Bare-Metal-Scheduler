#ifndef UART_UART_H_
#define UART_UART_H_

#include <stdint.h>

/*
	PUBLIC FUNCTION PROTOTYPES
*/
void init_uart(void);
void uart_write_char(char c);
void uart_write_string(char *str);
void uart_write_int(int value);


/*
	OTHER CONSTANTS
*/
#define RCC_BASE_ADDRESS 0x40023800U
#define GPIOA_BASE_ADDRESS 0x40020000U
#define USART2_BASE_ADDRESS 0x40004400U

#define f_CK 16000000U
#define BAUD_RATE 115200U
#define BRR_VALUE ((f_CK + (BAUD_RATE / 2U)) / BAUD_RATE) // (BAUD_RATE / 2U) rounds to nearest int

#endif
