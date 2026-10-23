# Bare-Metal Traffic Control System on STM32F407

A priority-based preemptive task scheduler written from scratch for the STM32F407ZGT6, running a dual-intersection traffic light controller as its workload.

**No HAL, no CMSIS, no RTOS.** Every register access is derived from RM0090 or the Cortex-M4 Generic User Guide.

<img width="2000" height="1500" alt="image" src="https://github.com/user-attachments/assets/a316fa66-dcac-43d7-a523-1f378c9b1413" />


## Features

- Preemptive priority scheduler: **SysTick** tick, **PendSV** context switch in naked assembly
- Hand-built initial stack frames per AAPCS
- `task_delay()` with `BASEPRI`-based critical sections
- Per-task stack overflow detection
- Watchdog task: detects deadline misses, forces both intersections to a safe RED state
- Fault injection and pedestrian button via EXTI
- Interrupt-driven UART RX (ring buffer) with a diagnostic shell
- Static allocation only

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

## References

RM0090 · Cortex-M4 Generic User Guide · AAPCS
