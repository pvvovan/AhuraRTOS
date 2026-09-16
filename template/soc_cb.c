/**
 * @file soc_cb.c
 * @brief Template for the SoC-owned kernel callbacks (_cb functions), for targets with no SoC
 *        package under soc/.
 *
 * COPY THIS ONLY IF YOUR TARGET HAS NO SoC PACKAGE. Every callback here is a fact about the
 * silicon rather than a product decision, so a package under soc/<vendor>/<family>/ already
 * implements the whole group for the parts it covers - see doc/soc.md, which also lists what is
 * packaged today. The installers never copy this file, and a project that selects a package with
 * AHURA_SOC must not either: the callbacks below are defined strong, so a copy next to a package
 * fails to link with a multiple-definition error naming the callback. That is the point.
 *
 * So this file is the escape hatch that keeps every other MCU supported: an unpackaged part, a
 * custom ASIC, an FPGA soft core. Copy it into the application source tree as soc_cb.c, add
 * that copy to the application build, and fill in the bodies against the target's own registers.
 * When it works, a package under soc/ is the natural next step - it is this file plus a
 * soc.cmake, and it makes the port reusable.
 *
 * Some of these are MANDATORY when their feature is enabled - the kernel declares them and
 * defines nothing, so a missing one is a link error rather than a silently empty hook. Each
 * definition says which it is. The #if guards match the exact condition under which the kernel
 * calls the group, so the file compiles cleanly under any configuration.
 *
 * Two are weak on purpose: os_tickless_pre_sleep_cb() and os_tickless_post_sleep_cb() are the
 * application's to replace (doc/tickless.md, "Application hooks"), so a strong definition in its
 * own sources wins over the defaults here.
 *
 * Nothing to implement for the CPU clock: the kernel reads CMSIS SystemCoreClock. A device whose
 * startup code lacks that symbol defines it anywhere in the application and keeps it current when
 * the clock tree changes.
 *
 * The application's own callbacks - os_log_output_cb, os_assert_failed_cb, os_stack_overflow_cb -
 * are NOT here. They live in template/os_cb.c, which every project copies.
 *
 * @copyright (c) 2026 Ahura Project Contributors
 *            SPDX-License-Identifier: GPL-3.0-or-later
 *            See LICENSE in the project root for the full license text.
 */

/*
 * ***********************************************************************************************************
 * Includes
 * ***********************************************************************************************************
*/

#include "ahura.h"

/*
 * ***********************************************************************************************************
 * Public function implementations
 * ***********************************************************************************************************
*/

#if (OS_CONFIG_TICK_SOURCE == OS_CONFIG_TICK_SOURCE_EXTERNAL)
/******************************************************************************************************/
/**
 * @brief Start the timer that drives the kernel tick, and arrange for its ISR to call
 *        os_tick_handler() OS_CONFIG_TICK_HZ times per second.
 *
 * REQUIRED while OS_CONFIG_TICK_SOURCE is OS_CONFIG_TICK_SOURCE_EXTERNAL: the kernel ships no
 * default, so leaving this out is a link error rather than a kernel whose clock never advances.
 * Delete this block (and set the option back to SYSTICK) to let the port program SysTick itself.
 *
 * Called once from os_init(), after the application has configured its clock tree. Give the
 * interrupt it starts the LOWEST priority the device offers, and keep it reachable by the kernel's
 * mask: with a nonzero OS_CONFIG_MAX_SYSCALL_IRQ_PRIORITY, a tick ISR above that threshold is
 * trapped by os_arch_isr_priority_check the moment it calls into the kernel.
 *
 * Example - an RTC/LPTIM-style peripheral, the usual reason to be here (Nordic nRF5x, and any
 * design whose SysTick stops in the sleep mode it ships with):
 *
 *     void os_arch_tick_init_cb(void)
 *     {
 *         my_lptim_start_periodic(OS_CONFIG_TICK_HZ);
 *         my_lptim_irq_priority_set(MY_LOWEST_IRQ_PRIORITY);
 *         my_lptim_irq_enable();
 *     }
 *
 *     void LPTIM_IRQHandler(void)   // the device's own vector name
 *     {
 *         my_lptim_flag_clear();
 *         os_tick_handler();
 *     }
 *
 * @return None.
 */
void os_arch_tick_init_cb(void)
{
}
#endif /* OS_CONFIG_TICK_SOURCE_EXTERNAL */

#if (OS_CONFIG_TRUSTZONE == OS_CONFIG_TRUSTZONE_NON_SECURE)
/******************************************************************************************************/
/**
 * @brief Bank the secure-side context (secure stack / PSP_S) of the task being switched out.
 *
 * Typically calls a secure gateway (cmse_nonsecure_entry) provided by the secure firmware.
 *
 * REQUIRED while OS_CONFIG_TRUSTZONE is OS_CONFIG_TRUSTZONE_NON_SECURE: the kernel ships no
 * default, so leaving this out is a link error rather than tasks switching with their secure
 * state left behind.
 *
 * @param[in] task_id  Kernel id of the task being switched out; 0 is the idle task, which never
 *                     owns a secure context.
 * @return None.
 */
void os_arch_tz_context_save_cb(uint32_t task_id)
{
    (void)task_id;
}

/******************************************************************************************************/
/**
 * @brief Restore the secure-side context of the task being switched in.
 *
 * REQUIRED on the same terms as os_arch_tz_context_save_cb().
 *
 * @param[in] task_id  Kernel id of the task being switched in; 0 is the idle task.
 * @return None.
 */
void os_arch_tz_context_restore_cb(uint32_t task_id)
{
    (void)task_id;
}
#endif /* OS_CONFIG_TRUSTZONE_NON_SECURE */

#if (OS_CONFIG_CORE_COUNT > 1U)
/******************************************************************************************************/
/**
 * @brief Return the index of the calling core, 0-based. SoC-specific: SIO CPUID on the RP2040.
 *
 * REQUIRED when OS_CONFIG_CORE_COUNT is above 1.
 *
 * @return uint32_t  The calling core's index.
 */
uint32_t os_arch_core_id_get_cb(void)
{
    return 0U;
}

#if (OS_CONFIG_TICKLESS_ENABLE != 1U)
/******************************************************************************************************/
/**
 * @brief Interrupt another core so it re-evaluates scheduling. SoC-specific: the RP2040's
 *        inter-core FIFO, the RP2350's doorbell.
 *
 * OPTIONAL while ticking is continuous: without an implementation the target core reacts at its
 * own next tick instead, which costs latency and nothing else.
 *
 * REQUIRED once OS_CONFIG_TICKLESS_ENABLE is on, which is why this stub is not offered in that
 * combination. A suppressed window is precisely the absence of a next tick, and this call is the
 * only thing that pulls core 0 out of a window a deadline armed on another core now falls inside.
 *
 * @param[in] core_id  Core to interrupt.
 * @return None.
 */
void os_arch_core_ipi_request_cb(uint32_t core_id)
{
    (void)core_id;
}
#endif /* OS_CONFIG_TICKLESS_ENABLE */

/******************************************************************************************************/
/**
 * @brief Boot a secondary core so it reaches os_core_start(). Called by os_start(), once per core
 *        from 1 to OS_CONFIG_CORE_COUNT-1, with the kernel complete and already running.
 *
 * REQUIRED when OS_CONFIG_CORE_COUNT is above 1: the kernel ships no default, so leaving this out
 * is a link error rather than a second core that silently never starts.
 *
 * Two things the implementation owes the kernel, in this order:
 *
 *   1. Point the core at a vector table whose context-switch entry is the kernel's handler. On
 *      Cortex-M that is PendSV, and using core 0's own table is the usual answer.
 *   2. Have the core call os_core_start(), which configures its banked per-core state and enters
 *      the scheduler. It does not return.
 *
 * Do NOT start the core any earlier than this callback fires: os_core_start() begins dispatching
 * immediately, and a core released during os_init() would pick from ready lists still being built.
 *
 * Example - the RP2040/RP2350, whose SDK boots core 1 onto core 0's vector table for you:
 *
 *     static void core1_entry(void)
 *     {
 *         my_core_local_irq_setup();   // anything banked per core
 *         os_core_start();             // does not return
 *     }
 *
 *     void os_arch_core_launch_cb(uint32_t core_id)
 *     {
 *         (void)core_id;               // only one secondary core on this part
 *         multicore_launch_core1(core1_entry);
 *     }
 *
 * @param[in] core_id  Core to start.
 * @return None.
 */
void os_arch_core_launch_cb(uint32_t core_id)
{
    (void)core_id;
}

/******************************************************************************************************/
/**
 * @brief Top of the given core's handler (MSP) stack. The first context switch on each core
 *        resets its MSP to this value while abandoning the boot context.
 *
 * REQUIRED when OS_CONFIG_CORE_COUNT is above 1: the vector table only names core 0's initial stack
 * pointer, and a secondary core reading it would share core 0's handler stack, both cores'
 * exception frames overwriting each other.
 *
 * Return the address a full stack pointer STARTS at (one past the region's highest byte), from the
 * symbols of the per-core stacks only the linker script knows:
 *
 *     uint32_t os_arch_handler_stack_top_cb(uint32_t core_id)
 *     {
 *         return (core_id == 1U)
 *              ? (uint32_t)&__StackOneTop    // one symbol pair per core
 *              : (uint32_t)&__StackTop;
 *     }
 *
 * @param[in] core_id  Core whose handler stack is asked for.
 * @return uint32_t  Top of that core's handler stack.
 */
uint32_t os_arch_handler_stack_top_cb(uint32_t core_id)
{
    (void)core_id;
    return 0U;
}

/* os_arch_handler_stack_limit_cb() is not offered here: the ARMv8-M port ships a default that is
 * right for core 0, and a stub would replace it. A multi-core ARMv8-M target defines it, returning
 * each core's stack bottom; the other ports never call it. */

/* The exact condition under which the kernel routes its spinlock through these callbacks
 * (os_arch_port_common.h): cores without LDREX/STREX, plus any core where
 * OS_CONFIG_SPINLOCK_SOC_BACKEND opts out of the built-in backend. */
#if (OS_ARCH_SPINLOCK_USE_CB)
/******************************************************************************************************/
/**
 * @brief Take the kernel spinlock, busy-waiting until it is free. Route it to the SoC's hardware
 *        spinlocks (e.g. RP2040 SIO). Called with interrupts already masked.
 *
 * MANDATORY when the built-in LDREX/STREX backend is unavailable or opted out of: the kernel
 * ships no default, so leaving it out fails at link time.
 *
 * @param[in,out] lock  The kernel's spinlock object.
 * @return None.
 */
void os_arch_spinlock_acquire_cb(os_arch_spinlock_t *lock)
{
    (void)lock;
}

/******************************************************************************************************/
/**
 * @brief Release the kernel spinlock taken by os_arch_spinlock_acquire_cb().
 *
 * MANDATORY on the same terms.
 *
 * @param[in,out] lock  The kernel's spinlock object.
 * @return None.
 */
void os_arch_spinlock_release_cb(os_arch_spinlock_t *lock)
{
    (void)lock;
}
#endif /* OS_ARCH_SPINLOCK_USE_CB */
#endif /* OS_CONFIG_CORE_COUNT > 1U */

#if (OS_CONFIG_TICKLESS_ENABLE == 1U)
/* The two tickless hooks. MANDATORY to exist while tickless idle is on, since the kernel defines
 * neither, and weak because they are the application's to replace.
 *
 * Both run with the kernel's interrupts masked. Polling a hardware flag is fine; waiting on
 * anything an interrupt must deliver (a DMA callback, HAL_GetTick()) hangs. Keep both short: their
 * duration is added to interrupt latency. A vendor HAL driving its own periodic tick is worth
 * suspending in the pre-sleep hook, or it ends every window at its own period. */

/******************************************************************************************************/
/**
 * @brief Called right before the idle sleep: select the sleep mode (e.g. SLEEPDEEP), gate clocks.
 *
 * Empty body = plain SLEEP (SLEEPDEEP left clear): the CPU clock stops but every peripheral clock -
 * UARTs, timers, DMA - keeps running, so nothing needs saving here and os_tickless_post_sleep_cb()
 * has nothing to restore. To guarantee a peripheral finishes before the CPU naps (a debug UART's
 * last line), block on its busy/TX-complete flag here. A deeper mode (STOP/SLEEPDEEP) gates
 * peripheral and system clocks: restore them, re-running the clock configuration if PLL/HSE were
 * affected, in os_tickless_post_sleep_cb() before anything relies on them again.
 *
 * @return None.
 */
OS_WEAK void os_tickless_pre_sleep_cb(void)
{
}

/******************************************************************************************************/
/**
 * @brief Called right after wakeup: clear SLEEPDEEP, restore clocks.
 *
 * Runs with the kernel's interrupts still masked and before the sleep has been announced, so the
 * kernel clock is still short by the whole sleep while this executes. Restore hardware here; do not
 * call kernel APIs that block, delay, or read the tick expecting it to be current.
 *
 * @return None.
 */
OS_WEAK void os_tickless_post_sleep_cb(void)
{
}
#endif /* OS_CONFIG_TICKLESS_ENABLE */
