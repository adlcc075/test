/*=================================================================================

    Module Name:  hal_gpmi.c

    General Description: GPMI hardware drivers and nand timings

===================================================================================
                               Freescale Confidential Proprietary
                        Template ID and version: IL93-TMP-01-0106  Version 1.20
                        (c) Copyright Freescale, All Rights Reserved

                                Copyright: 2007-2014 FREESCALE, INC.
                   All Rights Reserved. This file contains copyrighted material.
                   Use of this file is restricted by the provisions of a
                   Freescale Software License Agreement, which has either
                   accompanied the delivery of this software in shrink wrap
                   form or been expressly executed between the parties.

Revision History:
                            Modification     Tracking
Author (core ID)                Date          Number     Description of Changes
-------------------------   ------------    ----------   --------------------------
Terry Xie                   18/11/2014                   Transplant to rom utility
Shirley Ye                  11/May/2011     ENGR142968   Make BBM swap disabled at default
Shirley Ye                  17/Aug/2010     ENGR125745   added parameter for bbm byte position in metadata
Boris Shulman               15/Jul/2010     ENGR00125192 Arik Environment Setup
Fareed Mohammed             24/Mar/2010     ENGR00121599 Fixed toggle mode issues, ecc types mapping and merged 
                                                            dev_b06314_er3 branch for engr121764 fixes
Fareed Mohammed             17/Mar/2010     ENGR00121600 Fixed ROM fuse allocation to match with fixes done to sbmr mapping 
                                                            and ocotp allocation in rtl release 7.05.
Fareed Mohammed             03/09/2010      ENGR00118666 Added code to update toggle mode DDR nand timings from FCB
Geo McCullough (r9aadu)     08-12-2009      ENGR00118662 Removed ritaproto because N/A for codex
Yaniv Adiri (rm96583)       03-12-2009      ENGR00118867 Updated driver
Yaniv Adiri   (rm96583)     06-11-2009      ENGR00116995 Updated code for redundant NAND compatability
Boris Shulman (rm96330)     10-08-2009      ENGR00111570 Changes for Rita clock requirements
Boris Shulman (rm96330)     11-01-2009      ENGR00111569 Updated Rita SBMR definitions
Boris Shulman (rm96330)     06-01-2009      ENGR00111567 Added iomux configuration for Rita 
Jameer B Mulani (B11771)	03/12/2007	    ENGcm02114   Changes done for reading rest of boot image from 
											   		     block1

Jameer B Mulani (B11771)	07/11/2007	    ENGcm01173   Changes done for NFCv2

Jameer B Mulani(B11771)     19/09/2007      ENGcm00142   Added nand_data_read_next_boot_block 
		  												 function 

Surender Kumar(r66464)      24/08/1007      TLSbo95816   EDC Update
Surender Kumar (r66464)     21/05/2007      TLSbo92398 	 Initial implementation.

Portability: Portable to other compilers or platforms.

===================================================================================
                                        INCLUDE FILES
==================================================================================*/
#include "timer/timer.h"
#include "regsgpmi.h"
#include "hal_gpmi.h"

extern uint32_t regs_GPMI_base;
/*===================================================================================
                                        LOCAL FUNCITONS AND VARIABLES
==================================================================================*/
static int gpmi_find_cycles(U32 nand_time_ns, U32 gpmi_period_ns, U32 max_search_times)
{
    int i;
    U32 cycle_time = gpmi_period_ns;

    /* Assume a maximum of 15 tests */
    for (i = 1; i < max_search_times; i++) {
        if (cycle_time > nand_time_ns) {
            break;
        } else {
            cycle_time += gpmi_period_ns;
        }
    }

    return i;
}

void gpmi_set_nand_timing(NAND_Timing_t * nand_timing, U32 gpmi_period_ns)
{
    NAND_Timing_t *timing = (NAND_Timing_t *) nand_timing;
    U32 addr_setup_cycles;
    U32 data_setup_cycles;
    U32 data_hold_cycles;
    U32 busy_timeout;

    /* use default gpmi timing for a 24MHz clock if gpmi_period_ns is 0. */
    if (gpmi_period_ns == 0)
        gpmi_period_ns = GPMI_DEFAULT_CLK_PERIOD;

    /*
     * Set GPMI_TIMING0 and GPMI_TIMING1
     * Setup pin timing parameters: ADRESS_SETUP, DATA_SETUP, and DATA_HOLD.
     * Note that these are in units of GPMICLK cycles.
     */
    addr_setup_cycles = gpmi_find_cycles(timing->m_u8AddressSetup,
                                         gpmi_period_ns, NORMAL_CLOCKS_SEARCH_CYCLES);
    data_setup_cycles = gpmi_find_cycles(timing->m_u8DataSetup,
                                         gpmi_period_ns, NORMAL_CLOCKS_SEARCH_CYCLES);
    data_hold_cycles = gpmi_find_cycles(timing->m_u8DataHold,
                                        gpmi_period_ns, NORMAL_CLOCKS_SEARCH_CYCLES);
    busy_timeout = gpmi_find_cycles(FLASH_BUSY_TIMEOUT_DIV_4096,
                                    gpmi_period_ns, BUSY_TIMEOUT_SEARCH_CYCLES);

    HW_GPMI_TIMING0_WR(NAND_GPMI_TIMING0(addr_setup_cycles, data_setup_cycles, data_hold_cycles));
    HW_GPMI_TIMING1_WR(BF_GPMI_TIMING1_DEVICE_BUSY_TIMEOUT(busy_timeout));  // Number of cycles / 4096.
}

/*==========================================================================
FUNCTION: gpmi_poll_debug
DESCRIPTION:
       polling the GPMI_DEBUG register within a period of time, to check if it matches the expectant value .
==============================================================================*/
U32 gpmi_poll_debug(U32 mask, U32 match, U32 timeout)
{
    U32 ticks = 1;

	timeout = (timeout << 5) / 1000;
	GPT_delay(timeout);

    while ((!GPT_status())) {
        if ((HW_GPMI_DEBUG.U & mask) == (match & mask)) {
            return ticks;
        }

        ticks = ticks + 1;
    }

	GPT_disable();

    return 0;
}
