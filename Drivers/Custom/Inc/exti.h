#ifndef EXTI_EXTI_H_
#define EXTI_EXTI_H_

#include <stdint.h>

void init_exti(void);

extern volatile uint8_t fault_injection_requested;

#endif
