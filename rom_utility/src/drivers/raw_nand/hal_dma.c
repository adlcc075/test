/*=================================================================================

    Module Name:  hal_dma.c

    General Description: Limited IROM NFC Driver.

===================================================================================
                               Freescale Confidential Proprietary
                        Template ID and version: IL93-TMP-01-0106  Version 1.20
                        (c) Copyright Freescale, All Rights Reserved

                        Copyright: 2004-2014 FREESCALE, INC.
                   All Rights Reserved. This file contains copyrighted material.
                   Use of this file is restricted by the provisions of a
                   Freescale Software License Agreement, which has either
                   accompanied the delivery of this software in shrink wrap
                   form or been expressly executed between the parties.

Revision History:
                           Modification     Tracking
Author (core ID)               Date         Number      Description of Changes
-------------------------   ------------   ----------   --------------------------
Terry Xie                   18/11/2014                   Transplant to rom utility
Fareed Mohammed             24-Mar-2010    ENGR00121599 Removed unused stuff and fix dma code for toggle mode NAND device.
Fareed Mohammed             09-Mar-2010    ENGR00118666 Added boot support for toggle mode DDR NAND

Portability: Portable to other compilers or platforms.

===================================================================================
                                        INCLUDE FILES
==================================================================================*/
#include "hal_dma.h"
#include "timer/timer.h"

extern uint32_t regs_APBH_DMA_base;

/*===================================================================================
                                        GLOBAL FUNCTIONS
==================================================================================*/
/**
  * dma_start - kicks off a DMA transaction and records the start time
  * @pDmaChain: DMA chain to load and start
  * @cs: nand chip select
  *
  */
void dma_start(void *pDmaChain, U32 cs)
{
    reg32_t channel_mask = (0x1 << (NAND0_APBH_CH + cs));

    // soft reset dma chan, load cmd pointer and inc semaphore
    BW_APBH_CHANNEL_CTRL_RESET_CHANNEL(channel_mask);
    // Clear IRQ
    HW_APBH_CTRL1_CLR(channel_mask << BP_APBH_CTRL1_CH0_CMDCMPLT_IRQ);
    // Initialize DMA by setting up NextCMD field
    HW_APBH_CHn_NXTCMDAR_WR(NAND0_APBH_CH + cs, (reg32_t) pDmaChain);
    // Start DMA by incrementing the semaphore.
    BW_APBH_CHn_SEMA_INCREMENT_SEMA(NAND0_APBH_CH + cs, 1);
}

/** 
  * dma_wait - sends spins until the DMA is finished or times out
  * @timeout_us: How many microseconds to wait before timing out.
  * @cs: nand chip select
  *
  * return TRUE if succeed, otherwise return FALSE
  */
BOOL dma_wait(U32 timeout_us, U32 cs)
{
    U32 complete = 0;
    reg32_t channel_mask = (0x1 << (NAND0_APBH_CH + cs));
	timeout_us = (timeout_us << 5) / 1000;

	GPT_delay(timeout_us);

	/* spin until dma complete or timeout */
    while ((!GPT_status()) && (complete == 0)) {
        complete = (HW_APBH_CTRL1_RD() & (channel_mask));
    }

	GPT_disable();

    /* if timeout return error, else return NXTCMDAR field from last DMA command */
    if (complete == 0) {
        BW_APBH_CHANNEL_CTRL_RESET_CHANNEL(channel_mask); // abort dma by resetting channel
        return FALSE;
    } else {
        return TRUE;
    }
}

