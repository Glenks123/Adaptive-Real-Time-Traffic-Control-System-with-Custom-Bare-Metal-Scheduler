#include <stdint.h>
#include <stdio.h>
#include "../Inc/gpio.h"
#include "../../../Core/Inc/registers.h"
#include "../../../App/Inc/intersection.h"

void init_gpio(void) {
	// Enabling clock to GPIOD
	uint32_t *RCC_AHB1ENR = (uint32_t*) (RCC_BASE_ADDRESS + 0x30);
	*RCC_AHB1ENR |= (1 << 3);

	// Configure LED pins (PD0-PD5) for OUTPUT
	uint32_t *GPIOD_MODER = (uint32_t*) (GPIOD_BASE_ADDRESS + 0x00);
	*GPIOD_MODER &= ~(0xFFF); // clear
	*GPIOD_MODER |= (0x555); //set
}

void set_intersection_a(TrafficState state) {
	uint32_t *GPIOD_ODR = (uint32_t*) (GPIOD_BASE_ADDRESS + 0x14);

	// RED --> PD0 | AMBER --> PD1 | GREEN --> PD2
	switch (state) {
	case RED:
		// ensure AMBER and GREEN are turned off; Turn on RED
		*GPIOD_ODR = (*GPIOD_ODR & ~(0x7)) | 0x1;
		break;
	case GREEN:
		// ensure RED and AMBER are turned off; Turn on GREEN
		*GPIOD_ODR = (*GPIOD_ODR & ~(0x7)) | 0x4;
		break;
	case AMBER:
		// ensure RED and GREEN are turned off; Turn on AMBER
		*GPIOD_ODR = (*GPIOD_ODR & ~(0x7)) | 0x2;
		break;
	}
}

void set_intersection_b(TrafficState state) {
	uint32_t *GPIOD_ODR = (uint32_t*) (GPIOD_BASE_ADDRESS + 0x14);

	// RED --> PD3 | AMBER --> PD4 | GREEN --> PD5
	switch (state) {
	case RED:
		// ensure AMBER and GREEN are turned off; Turn on RED
		*GPIOD_ODR = (*GPIOD_ODR & ~(0x38)) | 0x08;
		break;
	case GREEN:
		// ensure RED and AMBER are turned off; Turn on GREEN
		*GPIOD_ODR = (*GPIOD_ODR & ~(0x38)) | 0x20;
		break;
	case AMBER:
		// ensure RED and GREEN are turned off; Turn on AMBER
		*GPIOD_ODR = (*GPIOD_ODR & ~(0x38)) | 0x10;
		break;
	}
}
