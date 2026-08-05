#ifndef INTERSECTION_INTERSECTION_H_
#define INTERSECTION_INTERSECTION_H_

#include <stdint.h>

void task1_interA_handler(void);
void task2_interB_handler(void);

typedef enum {
	RED, GREEN, AMBER
} TrafficState;

#define TASK_INTER_A_INDEX  1
#define TASK_INTER_B_INDEX  2
#define TASK_WATCHDOG_INDEX 3

/*
	DEFINING TASK DELAY TIMINGS & PADDINGS
*/
#define PHASE_RED_MS 7000U
#define PHASE_GREEN_MS 5000
#define PHASE_AMBER_MS 2000
#define WDG_PADDING 10

// We add padding because context switch doesn't happen exactly at tick 7000. It's delayed as we pend pendsv which waits for
// high priority tasks to finish before its handler is invoked.
#define MAX_BLOCK_TIME (PHASE_RED_MS + (WDG_PADDING * PHASE_RED_MS / 100))

#endif
