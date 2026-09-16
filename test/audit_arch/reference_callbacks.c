/**
 * @file reference_callbacks.c
 * @brief Deterministic SoC reference clock shared by the architecture regressions.
 *
 * A fixed 1 MHz source so a busy-wait or a tickless window has an epoch the test controls.
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

#include <stdint.h>

/*
 * ***********************************************************************************************************
 * Global variables
 * ***********************************************************************************************************
*/

uint64_t test_reference;

/*
 * ***********************************************************************************************************
 * Public function implementations
 * ***********************************************************************************************************
*/

/******************************************************************************************************/
/**
 * @brief The deterministic rate this regression measures against.
 *
 * @return What this stand-in reports.
 */
uint32_t os_arch_reference_clock_hz_cb(void)
{
    return 1000000U;
}

/******************************************************************************************************/
/**
 * @brief The rate a modeled tickless window is measured against.
 *
 * @return What this stand-in reports.
 */
uint32_t os_arch_tick_reference_clock_hz_cb(void)
{
    return 1000000U;
}

/******************************************************************************************************/
/**
 * @brief The modeled reference counter, advanced by the regression itself.
 *
 * @return What this stand-in reports.
 */
uint64_t os_arch_reference_clock_get_cb(void)
{
    return test_reference;
}
