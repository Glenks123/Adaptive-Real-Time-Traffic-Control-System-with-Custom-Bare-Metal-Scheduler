#ifndef SCHEDULER_SCHEDULER_H_
#define SCHEDULER_SCHEDULER_H_

#include <stdint.h>

/*
	DEFINING FUNCTION PROTOTYPES
*/
void enable_processor_faults(void);
void init_tasks_stacks(void);
__attribute__((naked)) void init_scheduler_stack(uint32_t sched_stack_start);
__attribute__((naked)) void switch_sp_to_psp(void);
void task_delay(uint32_t tick_count);


/*
	DEFINING STACK MEMORY MAP
*/
#define N_TASKS 5

#define SIZE_TASK_STACK 1024U
#define SIZE_SCHED_STACK 512U

#define SRAM_START 0x20000000
#define SRAM_SIZE ((128) * 1024)
#define SRAM_END ((SRAM_START) + (SRAM_SIZE))

#define SCHED_STACK_START SRAM_END
#define IDLE_STACK_START ((SRAM_END) - (SIZE_SCHED_STACK))
#define T1_STACK_START ((SRAM_END - SIZE_SCHED_STACK) - (SIZE_TASK_STACK))
#define T2_STACK_START ((SRAM_END - SIZE_SCHED_STACK) - (SIZE_TASK_STACK * 2))
#define T3_STACK_START ((SRAM_END - SIZE_SCHED_STACK) - (SIZE_TASK_STACK * 3))
#define T4_STACK_START ((SRAM_END - SIZE_SCHED_STACK) - (SIZE_TASK_STACK * 4))


/*
	DEFINING TASK STATES
*/
#define TASK_READY_STATE 0x00
#define TASK_BLOCKED_STATE 0x01
#define TASK_FAULT_STATE 0x02


/*
	DEFINING TASK CONTROL BLOCK
*/
typedef struct {
    uint32_t psp_value;          // Current PSP — saved on context switch
    uint32_t block_count;        // Tick count at which task unblocks
    uint8_t current_state;      // READY / BLOCKED / FAULT
    uint8_t priority;           // Higher number = higher priority
    uint32_t stack_boundary;     // Lowest valid PSP address — overflow detection
    uint32_t run_count;          // How many times this task has been scheduled
    uint32_t last_run_tick;      // Last tick this task ran — watchdog uses this
    uint32_t missed_deadlines;   // Incremented by watchdog on deadline miss
    void (*task_handler)(void);
} TCB_t;


/*
	OTHER CONSTANTS
*/
#define XPSR_VAL 0x01000000
#define EXC_RETURN_VAL 0xFFFFFFFD


#endif


