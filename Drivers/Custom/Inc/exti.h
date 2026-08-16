#ifndef EXTI_EXTI_H_
#define EXTI_EXTI_H_

#include <stdint.h>

void init_exti(void);

#define GPIOA_BASE_ADDRESS 0x40020000
#define EXTI_BASE_ADDRESS 0x40013C00

#endif
