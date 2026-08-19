#ifndef EXTI_EXTI_H_
#define EXTI_EXTI_H_

#include <stdint.h>

void init_exti(void);

#define BUTTON_DEBOUNCE_THRESHOLD_MS 50

extern volatile uint8_t fault_injection_requested;
extern volatile uint8_t pedestrian_crossing_requested;

#endif
