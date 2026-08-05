#ifndef GPIO_GPIO_H_
#define GPIO_GPIO_H_

#include <stdint.h>
#include "../../../App/Inc/intersection.h"

void init_gpio(void);
void set_intersection_a(TrafficState state);
void set_intersection_b(TrafficState state);

#define GPIOD_BASE_ADDRESS 0x40020C00

#endif
