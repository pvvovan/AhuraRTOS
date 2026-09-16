/**
 * @file os_arch_port.h
 * @brief Deterministic test port; only used by the standalone audit regression runner.
 * @copyright (c) 2026 Ahura Project Contributors
 *            SPDX-License-Identifier: GPL-3.0-or-later
 *            See LICENSE in the project root for the full license text.
 */

#ifndef OS_ARCH_PORT_H
#define OS_ARCH_PORT_H

/*
 * ***********************************************************************************************************
 * Includes
 * ***********************************************************************************************************
*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../../template/os_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * ***********************************************************************************************************
 * Macros
 * ***********************************************************************************************************
*/

#undef OS_CONFIG_CORE_COUNT
#define OS_CONFIG_CORE_COUNT 2U
#undef OS_CONFIG_TICKLESS_ENABLE
#define OS_CONFIG_TICKLESS_ENABLE 1U
#undef OS_CONFIG_TIMER_ENABLE
#define OS_CONFIG_TIMER_ENABLE 0U
#undef OS_CONFIG_LOG_ENABLE
#define OS_CONFIG_LOG_ENABLE 0U
#undef OS_CONFIG_TEST_ENABLE
#define OS_CONFIG_TEST_ENABLE 0U
#undef OS_CONFIG_ASSERT_ENABLE
#define OS_CONFIG_ASSERT_ENABLE 0U
#undef OS_CONFIG_STACK_CHECK_ENABLE
#define OS_CONFIG_STACK_CHECK_ENABLE 0U
#undef OS_CONFIG_STACK_WATERMARK_ENABLE
#define OS_CONFIG_STACK_WATERMARK_ENABLE 0U
#undef OS_CONFIG_TRUSTZONE
#define OS_CONFIG_TRUSTZONE 0U
#define OS_CONFIG_TRUSTZONE_NON_SECURE 2U
#define OS_ARCH_STACK_ALIGNMENT_BYTES 8U
#define __IO volatile
#define OS_INLINE static inline
#define OS_FORCE_INLINE static inline __attribute__((always_inline))
#define OS_WEAK __attribute__((weak))
#define OS_ARCH_IDLE() ((void)0)
#define OS_ARCH_CONTEXT_SWITCH_REQUEST() ((void)0)

#define OS_ARCH_SPINLOCK_INIT { 0U }
#define OS_ARCH_SLEEP(ticks) audit_sleep(ticks)

/*
 * ***********************************************************************************************************
 * Types
 * ***********************************************************************************************************
*/

/******************************************************************************************************/
typedef struct
{
    uint32_t locked;

} os_arch_spinlock_t;

/*
 * ***********************************************************************************************************
 * Public function prototypes
 * ***********************************************************************************************************
*/

/******************************************************************************************************/
/**
 * @brief Raise the kernel interrupt mask; returns the previous mask state for restore.
 */
uint32_t os_arch_kernel_mask_save(void);

/******************************************************************************************************/
/**
 * @brief Restore the kernel interrupt mask to a state returned by os_arch_kernel_mask_save.
 */
void os_arch_kernel_mask_restore(uint32_t mask);

/******************************************************************************************************/
/**
 * @brief Index of the calling core; always 0 on single-core builds.
 */
uint32_t os_arch_core_id_get(void);

/******************************************************************************************************/
/**
 * @brief Return true when executing in interrupt (handler) context.
 */
bool os_arch_in_isr(void);

/******************************************************************************************************/
/**
 * @brief Index of the highest set bit in a non-zero bitmap (the scheduler's ready-priority pick).
 *        One CLZ instruction on ARMv7-M and up; ARMv6-M has no CLZ, so GCC emits its small library
 *        routine there - still cheaper than scanning the task table.
 */
uint32_t os_arch_highest_bit_get(uint32_t bits);

/******************************************************************************************************/
/**
 * @brief Index of the lowest set bit in a non-zero bitmap (picks the IPI target from an affinity
 *        mask).
 */
uint32_t os_arch_lowest_bit_get(uint32_t bits);

/******************************************************************************************************/
/**
 * @brief Build the initial task stack frame for a newly created task.
 */
uint32_t *os_arch_task_stack_initialize(uint8_t *stack, size_t bytes,
                                      void (*entry)(void *), void *context);

/******************************************************************************************************/
/**
 * @brief Initialize architecture-specific low-level resources.
 */
void os_arch_init(void);

/******************************************************************************************************/
/**
 * @brief Start the kernel tick. See os_arch_port_common.h.
 */
void os_arch_tick_init(void);

/******************************************************************************************************/
/**
 * @brief Start the first task context. Does not return.
 */
void os_arch_start_first_task(void);

/******************************************************************************************************/
/**
 * @brief Trap for unrecoverable configuration faults detected at runtime; parks the core with all
 *        interrupts masked so a debugger lands right at the cause.
 */
void os_arch_config_fault_trap(void);

/******************************************************************************************************/
/**
 * @brief Interrupt another core so it re-evaluates scheduling. SoC-specific: the RP2040's
 *        inter-core FIFO, the RP2350's doorbell.
 */
void os_arch_core_ipi_request_cb(uint32_t core);

/******************************************************************************************************/
/**
 * @brief Boot a secondary core so it reaches os_core_start(). Called by os_start(), once per core
 *        from 1 to OS_CONFIG_CORE_COUNT-1, with the kernel complete and already running.
 */
void os_arch_core_launch_cb(uint32_t core);

/******************************************************************************************************/
/**
 * @brief Weak default for the idle wait: a plain WFI, which is correct on parts whose timers keep
 *        running through it.
 */
void os_arch_soc_idle_cb(void);

/******************************************************************************************************/
/**
 * @brief Acquire an inter-core spinlock (busy-waits; call with interrupts disabled). Compiles to
 *        nothing on single-core builds.
 */
void os_arch_spinlock_acquire(os_arch_spinlock_t *lock);

/******************************************************************************************************/
/**
 * @brief Release an inter-core spinlock. Compiles to nothing on single-core builds.
 */
void os_arch_spinlock_release(os_arch_spinlock_t *lock);

/******************************************************************************************************/
/**
 * @brief In BASEPRI mode, trap a kernel API call from an interrupt the kernel mask cannot reach
 *        (NVIC priority numerically below OS_CONFIG_MAX_SYSCALL_IRQ_PRIORITY): such a call could
 *        corrupt kernel state, so it parks in os_arch_config_fault_trap. Compiles to nothing in
 *        PRIMASK mode, where every interrupt is maskable.
 */
void os_arch_isr_priority_check(void);

/******************************************************************************************************/
/**
 * @brief Close one SysTick period on this core. Called by the kernel from every core's tick.
 */
void os_arch_cycle_tick(void);

/******************************************************************************************************/
/**
 * @brief Largest number of ticks this port can suppress in one window.
 */
uint32_t os_arch_max_suppressed_ticks_get(void);

/******************************************************************************************************/
/**
 * @brief Shortest window worth opening (contract in os_arch_port_common.h).
 */
uint32_t os_arch_min_suppressed_ticks_get(void);

/******************************************************************************************************/
/**
 * @brief Close the window: ask how long it really was, restore the tick, and account for it.
 */
uint32_t os_arch_elapsed_ticks_get(void);

/******************************************************************************************************/
/**
 * @brief Close out a tickless window: release anything the open took.
 */
void os_arch_sleep_finish(void);

/******************************************************************************************************/
/**
 * @brief Run one modeled tickless window of the requested length.
 */
void audit_sleep(uint32_t ticks);

/******************************************************************************************************/
/**
 * @brief Rate of the busy-wait counter, in Hz.
 */
uint32_t os_arch_delay_counter_hz_get(void);

/******************************************************************************************************/
/**
 * @brief The busy-wait counter itself: its low 32 bits.
 */
uint32_t os_arch_delay_counter_get(void);

#ifdef __cplusplus
}
#endif

#endif
