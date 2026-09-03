#ifndef UART_UART_H_
#define UART_UART_H_

#include <stdint.h>

/*
	PUBLIC FUNCTION PROTOTYPES
*/
void init_uart(void);
void uart_write_char(char c);
//void uart_write_keypress(uint32_t value);
void uart_write_string(char *str);
void uart_write_int(uint32_t value);
void uart_write_hex(uint32_t value);

extern volatile uint8_t rx_write_idx;
extern volatile uint8_t rx_buffer[64];
extern int16_t uart_read_byte(void);

/*s
	OTHER CONSTANTS
*/
#define RCC_BASE_ADDRESS 0x40023800U
#define GPIOA_BASE_ADDRESS 0x40020000U
#define USART2_BASE_ADDRESS 0x40004400U
#define NVIC_IPR_BASE_ADDRESS 0xE000E400U

// Implemented bits [7:4]. [3:0] are reserved
#define BASEPRI_THRESHOLD (2 << 4) // block priorities >= 2 i.e let priorities < 2 through

#define f_CK 16000000U
#define BAUD_RATE 115200U
#define BRR_VALUE ((f_CK + (BAUD_RATE / 2U)) / BAUD_RATE) // (BAUD_RATE / 2U) rounds to nearest int

#endif
