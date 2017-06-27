/*=================================================================================

    Module Name:  hal_bch.c

    General Description: BCH drivers

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
#include  "timer/timer.h"
#include  "regsbch.h"
#include "regsgpmi.h"
#include "hal_bch.h"

extern uint32_t regs_BCH_base;
extern uint32_t regs_GPMI_base;

/*==========================================================================
                                     Global FUNCTIONS
==========================================================================*/

/**
 * bch_update_ecc_param - update ecc parameters
 */
void bch_update_ecc_param(void)
{
    // set the BCH mode on/off.
    BW_GPMI_CTRL1_BCH_MODE(1);

    BW_BCH_MODE_ERASE_THRESHOLD(0);
}

/**
 * bch_clear_complete_flag - clear ecc complete flag
 */
void bch_clear_complete_flag(void)
{
    HW_BCH_CTRL_CLR(BM_BCH_CTRL_COMPLETE_IRQ);
}

/**
 * bch_set_flash_layout - set flash layout, including data block size, ecc level, bch type etc
 * @cs: nand chip select number
 * @bchcfg: BCH configuration information
 *
 * return TRUE if succeed, otherwise return FALSE
 */
BOOL bch_set_flash_layout(U32 cs, bchcfg_t *bchcfg)
{
    U32 ecc_level_block0;
    U32 ecc_level_blockn;

    /* detect whether input ecc level exceed the max ecc level */
    if ((BCH_ECC_LEVEL_MAX < bchcfg->block0_ecc_level)
        || (BCH_ECC_LEVEL_MAX < bchcfg->blockn_ecc_level)) {
        printf("ecc level exceed, max level is %d\n", BCH_ECC_LEVEL_MAX);
        return FALSE;
    } else {
        ecc_level_block0 = (bchcfg->block0_ecc_level >> 1);
        ecc_level_blockn = (bchcfg->blockn_ecc_level >> 1);
    }

    // for nand0
    /* set flash0layout0 bch ecc register */
    BW_BCH_FLASH0LAYOUT0_GF13_0_GF14_1(bchcfg->bch_type);
    BW_BCH_FLASH0LAYOUT0_NBLOCKS(bchcfg->blockn_num_per_page);
    BW_BCH_FLASH0LAYOUT0_META_SIZE(bchcfg->meta_sz);
    BW_BCH_FLASH0LAYOUT0_ECC0(ecc_level_block0);
    BW_BCH_FLASH0LAYOUT0_DATA0_SIZE((bchcfg->block0_sz) >> 2);

    /* set flash0layout1 bch ecc register */
    BW_BCH_FLASH0LAYOUT1_GF13_0_GF14_1(bchcfg->bch_type);
    BW_BCH_FLASH0LAYOUT1_PAGE_SIZE(bchcfg->total_page_sz);
    BW_BCH_FLASH0LAYOUT1_ECCN(ecc_level_blockn);
    BW_BCH_FLASH0LAYOUT1_DATAN_SIZE((bchcfg->blockn_sz) >> 2);

    return TRUE;
}

/**
 * bch_check_ecc_status - check ecc status
 * @cs: nand chip select number
 *
 * return true if errors are corrected, otherwise return false
 */
BOOL bch_check_ecc_status(U32 cs)
{
    BOOL ret = TRUE;
    BOOL is_timeout = TRUE;
    U32 ecc_status;
    U32 timeout_cycle = (ECC_STATUS_CHECK_TIMEOUT_MS << 5);		// we use a 32kHz timer for timeout, cycle = t * fclk

    /* spin until ECC Complete Interrupt triggers or timeout */
	GPT_delay(timeout_cycle);
    while (!GPT_status()) {
        if (HW_BCH_CTRL_RD() & BM_BCH_CTRL_COMPLETE_IRQ) {  // BCH completed interrupt
            is_timeout = FALSE;
            break;
        }
        if (HW_BCH_CTRL_RD() & BM_BCH_CTRL_BM_ERROR_IRQ) {  // AHB Bus interface Error
            is_timeout = FALSE;
            break;
        }
    }
	GPT_disable();

    if (is_timeout) {
		printf("check ecc timeout\n");
        return FALSE;
    }

    /* get bch ecc status from register */
    ecc_status = HW_BCH_STATUS0_RD();

    /* check ecc status */
    if (ecc_status & BM_BCH_STATUS0_ALLONES) {
        ret = FALSE;

    } else if (ecc_status & BM_BCH_STATUS0_UNCORRECTABLE) {
        ret = FALSE;

    } else if (ecc_status & BM_BCH_STATUS0_CORRECTED) {
        ret = TRUE;
    }

    /* Clear the ECC Complete IRQ. */
    BW_BCH_CTRL_COMPLETE_IRQ(0);

    return ret;
}

/*==================================================================================================

FUNCTION: void bch_reset(void)

DESCRIPTION:
    Soft reset BCH

ARGUMENTS PASSED:
    None

RETURN VALUE:
    None

PRE-CONDITIONS:
    None

POST-CONDITIONS:
    None

IMPORTANT NOTES:
    None
==================================================================================================*/
void bch_reset(void)
{
    /* Soft Reset the BCH block */
    BW_BCH_CTRL_SFTRST(BV_BCH_CTRL_SFTRST__RESET);
    BW_BCH_CTRL_SFTRST(BV_BCH_CTRL_SFTRST__RUN);

    /* bring out of reset and disable Clk gate. */
    BW_BCH_CTRL_CLKGATE(BV_BCH_CTRL_CLKGATE__NO_CLKS);
    BW_BCH_CTRL_CLKGATE(BV_BCH_CTRL_CLKGATE__RUN);
}

/**
 * bch_enable - enable BCH
 */
void bch_enable(void)
{
    /* Remove the clock gate. */
    HW_BCH_CTRL_CLR(BM_BCH_CTRL_CLKGATE);

    /*  Remove Soft Reset from the BCH block */
    HW_BCH_CTRL_CLR(BM_BCH_CTRL_SFTRST);
}
