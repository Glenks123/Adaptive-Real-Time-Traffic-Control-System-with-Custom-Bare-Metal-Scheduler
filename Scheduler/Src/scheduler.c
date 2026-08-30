#include <stdio.h>
#include <stdint.h>
#include "../Inc/scheduler.h"
#include "../Drivers/Custom/Inc/uart.h"
#include "../App/Inc/intersection.h"

void task0_idleTask_handler(void);
void task1_interA_handler(void);
void task2_interB_handler(void);
void task3_watchdog_handler(void);
void task4_uart_handler(void);

uint32_t retrieve_psp_value(void);
void save_psp_value(uint32_t current_psp_value);
void update_next_task(void);
void unblock_tasks(void);
void update_global_tick(void);
void schedule(void);
void SysTick_Handler(void);
__attribute__((naked)) void PendSV_Handler(void);
void check_stack_overflow(uint32_t current_psp);
void init_interrupt_priorities(void);

TCB_t user_tasks[N_TASKS];
uint32_t current_task = 0; // initialized to idle task as a safe default
uint32_t global_tick_count = 0;

void init_interrupt_priorities(void) {
	// Set priority level for pendsv to 15
	uint32_t *pSHPR3 = (uint32_t*) (0xE000ED20);
	*pSHPR3 &= ~(0xFUL << 20);
	*pSHPR3 |= (0xFUL << 20); // UL --> unsigned long: prevents signed integer overflow and ensures 32 bit container --> defensive programming
}

void enable_processor_faults(void) {
	/*
	 * In ARM Cortex M4, HardFault is enabledd by default but Usage, MemManage and Bus fault is disabled by default (escalated to
	 * HardFault as a result). Enabling them will allow for specificity wrt to debugging.
	 */
	uint32_t *pSHCSR = (uint32_t*) 0xE000ED24;

	*pSHCSR |= (1 << 18); // enable usage fault
	*pSHCSR |= (1 << 17); // enable bus fault
	*pSHCSR |= (1 << 16); // enable mem manage fault
}

void init_tasks_stacks(void) {
	/*
	 * Initializes user_tasks tcd struct and pushes SF1 (xPSR, PC, LR, R12, R0-R3)
	 *  and SF2 (R4-R11) to private stack of each task
	 */
	user_tasks[0].psp_value = IDLE_STACK_START;
	user_tasks[1].psp_value = T1_STACK_START;
	user_tasks[2].psp_value = T2_STACK_START;
	user_tasks[3].psp_value = T3_STACK_START;
	user_tasks[4].psp_value = T4_STACK_START;

	user_tasks[0].stack_boundary = IDLE_STACK_START - SIZE_TASK_STACK;
	user_tasks[1].stack_boundary = T1_STACK_START - SIZE_TASK_STACK;
	user_tasks[2].stack_boundary = T2_STACK_START - SIZE_TASK_STACK;
	user_tasks[3].stack_boundary = T3_STACK_START - SIZE_TASK_STACK;
	user_tasks[4].stack_boundary = T4_STACK_START - SIZE_TASK_STACK;

	user_tasks[0].current_state = TASK_READY_STATE;
	user_tasks[1].current_state = TASK_READY_STATE;
	user_tasks[2].current_state = TASK_READY_STATE;
	user_tasks[3].current_state = TASK_READY_STATE;
	user_tasks[4].current_state = TASK_READY_STATE;

	user_tasks[0].priority = 0;
	user_tasks[1].priority = 2;
	user_tasks[2].priority = 2;
	user_tasks[3].priority = 3;
	user_tasks[4].priority = 1;

	user_tasks[0].max_block_time = 200;
	user_tasks[1].max_block_time = MAX_BLOCK_TIME;
	user_tasks[2].max_block_time = MAX_BLOCK_TIME;
	user_tasks[3].max_block_time = 500;
	user_tasks[4].max_block_time = 2000;

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
		*pPSP = XPSR_VAL;

		// PC initialized to each task handler
		pPSP--;
		*pPSP = (uint32_t) user_tasks[i].task_handler;

		// LR set to EXC_RETURN value (returns CPU back to thread mode, and changes SP to PSP after context switch --> done by hardware)
		pPSP--;
		*pPSP = EXC_RETURN_VAL;

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
	 * we look at last_scheduled_tick, which tells us when last a task was scheduled (timestamp).
	 */
	int max_priority = -1;
	int priority_idx = 0;
	int last_run_count = 0xFFFFFFFF; // used to keep track of the lowest last_scheduled_tick among equal-priority tasks

	for (int i=0; i<N_TASKS; i++) {
		if (user_tasks[i].current_state == TASK_READY_STATE && i != 0) {
			if (user_tasks[i].priority > max_priority) {
				max_priority = user_tasks[i].priority;
				priority_idx = i;
				last_run_count = user_tasks[i].last_scheduled_tick;
			} else if (user_tasks[i].priority == max_priority) {
				if (user_tasks[i].last_scheduled_tick < last_run_count) {
					max_priority = user_tasks[i].priority;
					priority_idx = i;
					last_run_count = user_tasks[i].last_scheduled_tick;
				}
			}
		}
	}

	user_tasks[priority_idx].last_scheduled_tick = global_tick_count;
	current_task = priority_idx;
}

void check_stack_overflow(uint32_t current_psp) {
	if (current_psp < user_tasks[current_task].stack_boundary) {
		uart_write_string("[STACK OVERFLOW] Task ");
		uart_write_int(current_task);
		uart_write_string(" | PSP: ");
		uart_write_hex(current_psp);
		uart_write_string(".\r\n");
		while(1);
	}
}

__attribute__((naked)) void PendSV_Handler(void) {
	/*
	 *	Performs context saving (saving SF2 of current task) and context retrieval (fetching SF2 of next task to run).
	 *	Only performed once during boot
	 */
	__asm volatile("MRS R0, PSP"); // retrieving PSP of current task and storing in R0
	__asm volatile("PUSH {LR}"); // holds value of EXC_RETURN
	__asm volatile("BL check_stack_overflow");
	__asm volatile("MRS R0, PSP");
	__asm volatile("STMDB R0!, {R4-R11}"); // R0 holds the current PSP value
	__asm volatile("BL save_psp_value"); // current_psp_value receives R0 according to AAPCS. Saving PSP to global data section
	//  software record keeping of PSP value

	__asm volatile("BL update_next_task"); // decide the next task to run
	__asm volatile("BL retrieve_psp_value"); // get PSP of next task
	__asm volatile("LDMIA R0!, {R4-R11}"); // popping SF2 of next task from private stack
	__asm volatile("MSR PSP, R0"); // hardware register update of PSP register
	__asm volatile("POP {LR}");
	__asm volatile("BX LR");
}

void task_delay(uint32_t tick_count) {
	/*
	 * When this function is called, the task state is set to BLOCKED. For example, if we need to turn on an LED for 5000 ticks
	 * followed by another LED, we call this function in between task_delay(5000). We unblock a task when block count == global count.
	 * Therefore, global count acts as a reference reference point for when the blocked tasks' alarm has arrived
	 *
	 * ISSUE WITH CHANGING STATE FIRST FOLLOWED BY BLOCK COUNT:
	 * If systick fires between the two writes, unblock_tasks() may evaluate this task
	 * with BLOCKED state but block_count still 0, causing the task to be immediately
	 * unblocked and the delay never honoured. Interrupts are disabled for the duration
	 * of both writes to prevent this race condition. (A race condition is when the
	 * correctness of the result depends on the timing of two concurrent operations.)
	 *
	 * --> we either change the order of operation or just add __disable_irq() (__asm volatile ("CPSID I");
	 * ) and __enable_irq() (__asm volatile ("CPSIE I")) around
	 * for full protection
	 */

	//user_tasks[current_task].current_state = TASK_BLOCKED_STATE;
	//user_tasks[current_task].block_count = tick_count + global_tick_count;

	__asm volatile ("CPSID I"); // __disable_irq();
	user_tasks[current_task].block_count = tick_count + global_tick_count;
	user_tasks[current_task].current_state = TASK_BLOCKED_STATE;
	__asm volatile ("CPSIE I"); // __enable_irq();
}

void unblock_tasks(void){
	for (int i=0; i<N_TASKS; i++) {
		if (user_tasks[i].current_state == TASK_BLOCKED_STATE) {
			if (user_tasks[i].block_count == global_tick_count) {
				user_tasks[i].current_state = TASK_READY_STATE;
			}
		}
	}
}

void update_global_tick(void) {
	global_tick_count++;
}

void schedule(void) {
	/* Pends PendSV	exception --> toggling 28th bit of ICSR register*/
	uint32_t *pICSR = (uint32_t*) 0xE000ED04;
	*pICSR |= (1 << 28);
}

void SysTick_Handler(void) {
	/*
	 *	Update global tick counter, checks if any tasks need to be unblocked, then pends PendSV (schedule)
	 */
	update_global_tick();
	unblock_tasks();
	schedule();
}

void HardFault_Handler(void) {
	printf("Exception : Hardfault\n");
	while(1);
}

void MemManage_Handler(void) {
	printf("Exception : MemManage\n");
	while(1);
}

void BusFault_Handler(void) {
	printf("Exception : BusFault\n");
	while(1);
}
