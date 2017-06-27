/*
 * Copyright (C) 2008-2011, Freescale Semiconductor, Inc. All Rights Reserved
 * THIS SOURCE CODE IS CONFIDENTIAL AND PROPRIETARY AND MAY NOT
 * BE USED OR DISTRIBUTED WITHOUT THE WRITTEN PERMISSION OF
 * Freescale Semiconductor, Inc.
*/

/*!
 * @defgroup diag_util Diagnostic utilities such as timer, uart, etc.
 */

/*!
 * @file time.c
 * @brief Basic clock functions used by the ANSI C time (see time.h)
 *
 * @ingroup diag_util
 */

#include "platform.h"
#include "io.h"
#include "debug.h"
#include "timer.h"
#include "rom_symbols.h"


static volatile int clock_counter;

extern chip_e chip;

/* Return the current clock value */
/* In order to obtain a high precision, whilst keeping the performance  */
/* penalty low, IRQs are only occuring at a rate of 100 per second, but */
/* the current timer value is factored in.  */
unsigned int ref_clock(void)
{
    return 0xFFFFFFFF - readl(EPIT_BASE_ADDR + EPITCNR);
}

/*!
 * clock initialization - it is called even before main()
 * TODO: need to allow user to specific to use CKIL or IPG or IPG_CLK_HIGHFREQ
 *       for higher resolution
 */
void init_clock(u32 rate)
{
    // enable timer and start it with 32KHz input clock
    writel(0x00010000, EPIT_BASE_ADDR + EPITCR);

    // make sure reset is complete
    while ((readl(EPIT_BASE_ADDR + EPITCR) & 0x10000) != 0) ;
    writel(0, EPIT_BASE_ADDR + EPITCMPR);   // always compare with 0

    if (rate == 32768) {

        writel(0x030E0002, EPIT_BASE_ADDR + EPITCR);
        writel(0x030E0003, EPIT_BASE_ADDR + EPITCR);
    } else {
        DEBUG_LOG("TODO\n");
        while (1) {
        }
    }
}


void GPT_init()
{
    f_v_v gpt_init = NULL;

    gpt_init = (f_v_v)psp_rom_get_symble(ROM_SYM_f_GPT_init, chip);
    if (!gpt_init) {
        if (chip == CHIP_MX6SLL_TO1_0)
            return ;

        DEBUG_LOG("Failed to get address of GPT_init() func\n");
        return;
    }
    printf("gpt_init start\n");
    gpt_init();
	printf("gpt_init finished\n");
}

void GPT_delay(UINT32 delay)
{
    f_v_u32 gpt_delay = NULL;

    gpt_delay = (f_v_u32)psp_rom_get_symble(ROM_SYM_f_GPT_delay, chip);
    if (!gpt_delay) {
        DEBUG_LOG("Failed to get address of GPT_delay() func\n");
        return;
    }

    gpt_delay(delay);
}


BOOL GPT_status(void)
{
    f_b_v gpt_status = NULL;

    gpt_status = (f_b_v)psp_rom_get_symble(ROM_SYM_f_GPT_status, chip);
    if (!gpt_status) {
        DEBUG_LOG("Failed to get address of GPT_status() func\n");
        return FALSE;
    }

    return (gpt_status());
}

void GPT_reset_delay(UINT32 delay)
{
    f_v_u32 gpt_reset_delay = NULL;

    gpt_reset_delay = (f_v_u32)psp_rom_get_symble(ROM_SYM_f_GPT_reset_delay, chip);
    if (!gpt_reset_delay) {
        DEBUG_LOG("Failed to get address of GPT_reset_delay() func\n");
        return;
    }

    gpt_reset_delay(delay);
}


void GPT_disable(void)
{
    f_v_v gpt_disable = NULL;

    gpt_disable = (f_v_v)psp_rom_get_symble(ROM_SYM_f_GPT_disable, chip);
    if (!gpt_disable) {
        DEBUG_LOG("Failed to get address of GPT_disable() func\n");
        return;
    }

    gpt_disable();
}


//
// Delay for some number of micro-seconds
//
void hal_delay_us(unsigned int usecs)
{
    /*
     * This causes overflow.
     * unsigned int delayCount = (usecs * 32768) / 1000000;
     * So use the following one instead
     */
    unsigned int delayCount = (usecs * 512) / 15625;

    if (delayCount == 0) {
        return;
    }

    writel(0x01, EPIT_BASE_ADDR + EPITSR);  // clear the compare status bit

    writel(delayCount, EPIT_BASE_ADDR + EPITLR);

    while ((0x1 & readl(EPIT_BASE_ADDR + EPITSR)) == 0) ;   // return until compare bit is set
}

void hal_set_timer_timeout(unsigned int usecs)
{
    unsigned int delayCount = (usecs * 512) / 15625;

    if (delayCount == 0) {
        return;
    }

    writel(0x01, EPIT_BASE_ADDR + EPITSR);  // clear the compare status bit

    writel(delayCount, EPIT_BASE_ADDR + EPITLR);
}

BOOL hal_is_timer_timeout()
{
   return (0x01 == (0x1 & readl(EPIT_BASE_ADDR + EPITSR)));
}
