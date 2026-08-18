#include <stdint.h>
#include "../Inc/intersection.h"
#include "../../Drivers/Custom/Inc/gpio.h"
#include "../../Scheduler/Inc/scheduler.h"
#include "../../Drivers/Custom/Inc/uart.h"
#include "../../Drivers/Custom/Inc/exti.h"

volatile TrafficState intersection_a_state = RED;
volatile TrafficState intersection_b_state = RED;

void check_task_deadline(uint8_t task_index);

void task1_interA_handler(void) {
	while (1) {
		if (fault_injection_requested) {
			__asm volatile("CPSID I");
			uart_write_string("BUTTON FAULT INJECTION INITIAITED: TASK A FAULTED.\r\n");
			__asm volatile("CPSIE I");
			while (1) {
				// trapping the task here --> not updating last_tick_count which eventually triggers watchdog handler
			}
		}

		user_tasks[TASK_INTER_A_INDEX].last_run_tick = global_tick_count;

		intersection_a_state = RED;
		set_intersection_a(intersection_a_state);
		task_delay(PHASE_RED_MS); // block task for 7 seconds, so scheudler runs other tasks
		while(intersection_b_state != RED) {
			task_delay(100); //  yield for 100ms until  interB is RED so interA can transition to green
			user_tasks[TASK_INTER_A_INDEX].last_run_tick = global_tick_count;
		}
		user_tasks[TASK_INTER_A_INDEX].last_run_tick = global_tick_count;
		__asm volatile("CPSID I");
		uart_write_string("[TICK ");
		uart_write_int(global_tick_count);
		uart_write_string(" ] A -> RED");
		uart_write_string("\r\n");
		__asm volatile("CPSIE I");

		intersection_a_state = GREEN;
		set_intersection_a(intersection_a_state);
		task_delay(PHASE_GREEN_MS);
		user_tasks[TASK_INTER_A_INDEX].last_run_tick = global_tick_count;
		__asm volatile("CPSID I");
		uart_write_string("[TICK ");
		uart_write_int(global_tick_count);
		uart_write_string(" ] A -> GREEN");
		uart_write_string("\r\n");
		__asm volatile("CPSIE I");

		intersection_a_state = AMBER;
		set_intersection_a(intersection_a_state);
		task_delay(PHASE_AMBER_MS);
		user_tasks[TASK_INTER_A_INDEX].last_run_tick = global_tick_count;
		__asm volatile("CPSID I");
		uart_write_string("[TICK ");
		uart_write_int(global_tick_count);
		uart_write_string(" ] A -> AMBER");
		uart_write_string("\r\n");
		__asm volatile("CPSIE I");
	}
}

void task2_interB_handler(void) {
	while (1) {
		user_tasks[TASK_INTER_B_INDEX].last_run_tick = global_tick_count;

		intersection_b_state = RED;
		set_intersection_b(intersection_b_state);
		task_delay(PHASE_RED_MS); // block task for 7 seconds, so scheudler runs other tasks
		while(intersection_a_state != RED) {
			task_delay(100); //  yield for 100ms, then check again
			user_tasks[TASK_INTER_B_INDEX].last_run_tick = global_tick_count;
		}
		user_tasks[TASK_INTER_B_INDEX].last_run_tick = global_tick_count;
		__asm volatile("CPSID I");
		uart_write_string("[TICK ");
		uart_write_int(global_tick_count);
		uart_write_string(" ] B -> RED");
		uart_write_string("\r\n");
		__asm volatile("CPSIE I");

		intersection_b_state = GREEN;
		set_intersection_b(intersection_b_state);
		task_delay(PHASE_GREEN_MS);
		user_tasks[TASK_INTER_B_INDEX].last_run_tick = global_tick_count;
		__asm volatile("CPSID I");
		uart_write_string("[TICK ");
		uart_write_int(global_tick_count);
		uart_write_string(" ] B -> GREEN");
		uart_write_string("\r\n");
		__asm volatile("CPSIE I");

		intersection_b_state = AMBER;
		set_intersection_b(intersection_b_state);
		task_delay(PHASE_AMBER_MS);
		user_tasks[TASK_INTER_B_INDEX].last_run_tick = global_tick_count;
		__asm volatile("CPSID I");
		uart_write_string("[TICK ");
		uart_write_int(global_tick_count);
		uart_write_string(" ] B -> AMBER");
		uart_write_string("\r\n");
		__asm volatile("CPSIE I");
	}
}

void task3_watchdog_handler(void) {
	while (1) {
		static uint8_t first_run = 0; // invoking watchdog to skip first check right after boot
		if (first_run == 0) {
			first_run++;
		} else {
			check_task_deadline(TASK_INTER_A_INDEX);
			check_task_deadline(TASK_INTER_B_INDEX);
		}

		task_delay(500);
	}
}

void check_task_deadline(uint8_t task_index) {
	__asm volatile("CPSID I");
	uart_write_string("GLOBAL TICK COUNT: ");
	uart_write_int(global_tick_count);
	uart_write_string(" | LAST RUN TICK: ");
	uart_write_int(user_tasks[task_index].last_run_tick);
	uart_write_string(" | MAX BLOCK TIME: ");
	uart_write_int(user_tasks[task_index].max_block_time);
	uart_write_string("\r\n");
	__asm volatile("CPSIE I");

	if (((global_tick_count - user_tasks[task_index].last_run_tick) > user_tasks[task_index].max_block_time)
					&& (user_tasks[task_index].current_state != TASK_FAULT_STATE)) { // Fault handling
		// Force Fault state
		 user_tasks[task_index].current_state = TASK_FAULT_STATE;
		// Set LEDs to RED
		 set_intersection_a(RED);
		 set_intersection_b(RED);
		// Report via UART (We disable IRQs during uart write, to disable garbled output --> ensures no context switches
		// take place until the full string is completely out)
		__asm volatile ("CPSID I"); // __disable_irq();
		uart_write_string("[TICK ");
		uart_write_int(global_tick_count);
		uart_write_string("] FAULT INJECTION: INTERSECTION ");

		if (task_index == TASK_INTER_A_INDEX) {
			uart_write_string("A");
		} else if (task_index == TASK_INTER_B_INDEX) {
			uart_write_string("B");
		}

		uart_write_string(" STUCK\r\n");
		__asm volatile ("CPSIE I"); // __enable_irq();
		// Increment missed_deadlines counter
		 user_tasks[task_index].missed_deadlines += 1;
	}
}

