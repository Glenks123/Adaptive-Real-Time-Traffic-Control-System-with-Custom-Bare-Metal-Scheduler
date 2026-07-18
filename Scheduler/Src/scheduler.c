#include <stdio.h>
#include <stdint.h>
#include "scheduler.h"


void task0_idleTask_handler(void);
void task1_interA_handler(void);
void task2_interB_handler(void);
void task3_watchdog_handler(void);
void task4_uart_handler(void);

void init_tasks_stacks(void);
__attribute__((naked)) void init_scheduler_stack(uint32_t sched_stack_start);
uint32_t retrieve_psp_value(void);
__attribute__((naked)) void switch_sp_to_psp(void);
void save_psp_value(uint32_t current_psp_value);
void update_next_task(void);
__attribute__((naked)) void PendSV_Handler(void);

TCB_t user_tasks[N_TASKS];
uint32_t current_task = 0; // initialized to idle task as a safe default
uint32_t global_tick_count = 0;

void init_tasks_stacks(void) {
	user_tasks[0].psp_value = IDLE_STACK_START;
	user_tasks[1].psp_value = T1_STACK_START;
	user_tasks[2].psp_value = T2_STACK_START;
	user_tasks[3].psp_value = T3_STACK_START;
	user_tasks[4].psp_value = T4_STACK_START;

	user_tasks[0].current_state = TASK_READY_STATE;
	user_tasks[1].current_state = TASK_READY_STATE;
	user_tasks[2].current_state = TASK_READY_STATE;
	user_tasks[3].current_state = TASK_READY_STATE;
	user_tasks[4].current_state = TASK_READY_STATE;

	user_tasks[0].task_handler = task0_idleTask_handler;
	user_tasks[1].task_handler = task1_interA_handler;
	user_tasks[2].task_handler = task2_interB_handler;
	user_tasks[3].task_handler = task3_watchdog_handler;
	user_tasks[4].task_handler = task4_uart_handler;

	uint32_t *pPSP;
	for (int i=0; i<N_TASKS; i++) {
		pPSP = (uint32_t*) user_tasks[i].psp_value;

		// Initializing XPSR with t-bit of 1 (24th bit) --> thumb2 state
		pPSP--;
		*pPSP = 0x01000000;

		// PC initialized to each task handler
		pPSP--;
		*pPSP = (uint32_t) user_tasks[i].task_handler;

		// LR set to EXC_RETURN value (returns CPU back to thread mode, and changes SP to PSP after context switch)
		pPSP--;
		*pPSP = 0xFFFFFFFD;

		// initializing registers R0-R12 to 0
		for (int j=0; j<13; j++) {
			pPSP--;
			*pPSP = 0;
		}

		user_tasks[i].psp_value = (uint32_t) pPSP;
	}
}

__attribute__((naked)) void init_scheduler_stack(uint32_t sched_stack_start) {
	/*
	 * Points the MSP to the start of scheduler stack
	 */
	__asm volatile("MSR MSP, R0");
	__asm volatile("BX LR");
}

uint32_t retrieve_psp_value(void) {
	/*
	 * Returns PSP of current task
	 */
	return user_tasks[current_task].psp_value;
}

__attribute__((naked)) void switch_sp_to_psp(void) {
	/*
	 * Retrives PSP of current task, then loads it onto PSP register. Finally, setting SP to PSP by toggling SPSEL in CONTROL
	 */
	__asm volatile("PUSH {LR}");
	__asm volatile("BL retrieve_psp_value");
	__asm volatile("MSR PSP, R0");
	__asm volatile("POP {LR}");

	__asm volatile("MOV R0, #0x02");
	__asm volatile("MSR CONTROL, R0");
	__asm volatile("BX LR");
}

void save_psp_value(uint32_t current_psp_value) {
	user_tasks[current_task].psp_value = current_psp_value;
}

void update_next_task(void) {
	/*
	 * Priority based decision handling to decide which is the next task to run: Scans tasks with ready state,
	 * out of which chooses the highest priority task to run. If two tasks have equal priority and  ready state, then
	 * we look at last_run_tick, which tells us when last a task was scheduled (timestamp).
	 */
	int max_priority = -1;
	int priority_idx = 0;
	int last_run_count = 0xFFFFFFFF;

	for (int i=0; i<N_TASKS; i++) {
		if (user_tasks[i].current_state == TASK_READY_STATE && i != 0) {
			if (user_tasks[i].priority > max_priority) {
				max_priority = user_tasks[i].priority;
				priority_idx = i;
				last_run_count = user_tasks[i].last_run_tick;
			} else if (user_tasks[i].priority == max_priority) {
				if (user_tasks[i].last_run_tick < last_run_count) {
					max_priority = user_tasks[i].priority;
					priority_idx = i;
					last_run_count = user_tasks[i].last_run_tick;
				}
			}
		}
	}

	user_tasks[priority_idx].last_run_tick = global_tick_count;
	current_task = priority_idx;
}

__attribute__((naked)) void PendSV_Handler(void) {
	/*
	 *	Performs context saving (saving SF2 of current task) and context retrieval (fetching SF2 of next task to run)
	 */
	__asm volatile("MRS R0, PSP"); // retrieving PSP of current task and storing in R0
	__asm volatile("STMDB R0!, {R4-R11}"); // R0 holds the current PSP value
	__asm volatile("PUSH {LR}");
	__asm volatile("BL save_psp_value"); // current_psp_value receives R0 according to AAPCS. Saving PSP to global data section
	//  software record keeping of PSP value

	__asm volatile("BL update_next_task"); // decide the next task to run
	__asm volatile("BL retrieve_psp_value"); // get PSP of next task
	__asm volatile("LDMIA R0!, {R4-R11}"); // popping SF2 of next task from private stack
	__asm volatile("MSR PSP, R0"); // hardware register update of PSP register
	__asm volatile("POP {LR}");
	__asm volatile("BX LR");
}


