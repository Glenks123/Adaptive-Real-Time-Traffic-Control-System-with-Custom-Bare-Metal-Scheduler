# Bare-Metal Preemptive Scheduler on STM32F407

A priority-based preemptive task scheduler written from scratch for the STM32F407ZGT6, running a dual-intersection traffic light controller as its workload.

**No HAL, no CMSIS, no RTOS.** Every register access is derived from RM0090 or the Cortex-M4 Generic User Guide.

## Features

- Preemptive priority scheduler: **SysTick** tick, **PendSV** context switch in naked assembly
- Hand-built initial stack frames per AAPCS
- `task_delay()` with `BASEPRI`-based critical sections
- Per-task stack overflow detection
- Watchdog task: detects deadline misses, forces both intersections to a safe RED state
- Fault injection and pedestrian button via EXTI
- Interrupt-driven UART RX (ring buffer) with a diagnostic shell
- Static allocation only

## Architecture

```
SysTick ISR ──► tick++, unblock tasks, pend PendSV
                        │
PendSV ─────────► save R4-R11 → pick next task → restore R4-R11
                        │
        ┌───────────┬───┴───────┬──────────┐
     Inter. A    Inter. B    Watchdog   UART Shell
      (Pri 2)     (Pri 2)     (Pri 3)    (Pri 1)         Idle (Pri 0)
```

| Source | Priority |
|---|---|
| SysTick | 0 |
| USART2 | 1 |
| PendSV | 15 (lowest, never preempts another ISR) |

Equal-priority tasks are tie-broken on `last_scheduled_tick`.

## Hardware

STM32F407ZGT6 board, 6 LEDs, 2 buttons (10 kΩ pull-downs), 3.3 V USB-UART adapter.

| Signal | Pin |
|---|---|
| Intersection A R/A/G | PD0 / PD1 / PD2 |
| Intersection B R/A/G | PD3 / PD4 / PD5 |
| USART2 TX / RX | PA2 / PA3 |
| Fault injection (EXTI0) | PA0 |
| Pedestrian crossing (EXTI1) | PA1 |

## Shell (115200 8N1)

| Command | Output |
|---|---|
| `show uptime` | Ticks since boot |
| `show scheduler` | Current task, tick count, context switches |
| `show tasks` | State, priority, run count, missed deadlines |

## Build

STM32CubeIDE, plain C project. Register definitions live in `Core/Inc/registers.h`. Flash over ST-LINK and open a serial terminal on the UART adapter.

## Status

Scheduler, traffic logic, watchdog, EXTI and shell are working. Remaining: RM0090 comment pass and cleanup.

**Known issue:** after fault injection on Intersection A, Intersection B stays RED instead of resuming.

## Bugs and lessons

- **Watchdog blind spot:** the scheduler overwrote `last_run_tick`, hiding stalls. Fixed by splitting it into `last_scheduled_tick` (scheduler) and `last_run_tick` (task-owned).
- **First-switch hard fault:** the dummy xPSR needs the Thumb bit (`0x01000000`) and the stack must be 8-byte aligned.
- **EXTI_PR:** write-1-to-clear, so use `=`, not `|=`.

## Demo

<!-- TODO: video link -->

## References

RM0090 · Cortex-M4 Generic User Guide · AAPCS
