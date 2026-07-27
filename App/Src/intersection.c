#include <stdint.h>
#include "../Inc/intersection.h"
#include "../../Drivers/Custom/Inc/gpio.h"
#include "../../Scheduler/Inc/scheduler.h"

volatile TrafficState intersection_a_state = RED;
volatile TrafficState intersection_b_state = RED;

void task1_interA_handler(void) {
	while (1) {
		intersection_a_state = RED;
		set_intersection_a(intersection_a_state);
		task_delay(7000); // block task for 7 seconds, so scheudler runs other tasks
		while(intersection_b_state != RED) task_delay(100); //  yield for 100ms, then check again

		intersection_a_state = GREEN;
		set_intersection_a(intersection_a_state);
		task_delay(5000);

		intersection_a_state = AMBER;
		set_intersection_a(intersection_a_state);
		task_delay(2000);
	}
}

void task2_interB_handler(void) {
	while (1) {
		intersection_b_state = RED;
		set_intersection_b(intersection_b_state);
		task_delay(7000); // block task for 7 seconds, so scheudler runs other tasks
		while(intersection_a_state != RED) task_delay(100); //  yield for 100ms, then check again

		intersection_b_state = GREEN;
		set_intersection_b(intersection_b_state);
		task_delay(5000);

		intersection_b_state = AMBER;
		set_intersection_b(intersection_b_state);
		task_delay(2000);
	}
}

