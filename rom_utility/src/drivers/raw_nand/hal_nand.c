/*=================================================================================

    Module Name:  hal_nand.c

    General Description: nand IP drivers

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
		  												 function 
Portability: Portable to other compilers or platforms.

===================================================================================
                                        INCLUDE FILES
==================================================================================*/
#include "io.h"
#include "regsgpmi.h"
#include "regsbch.h"
#include "hal_dma.h"
#include "hal_gpmi.h"
#include "hal_bch.h"
#include "hal_nand.h"
#include "nand_base.h"

/*===================================================================================
                                        GLOBAL VARIABLES
==================================================================================*/
extern uint32_t regs_GPMI_base;
extern uint32_t regs_BCH_base;

/*===================================================================================
                                        GLOBAL FUNCTIONS
==================================================================================*/
/**
  * hal_nand_wait_for_ready - wait for ready
  * @cs: chip select
  */
void hal_nand_wait_for_ready(U32 cs)
{
    hw_gpmi_debug_t debug;
    hw_gpmi_ctrl0_t ctrl0;

    // Capture value of debug.
    debug = HW_GPMI_DEBUG;

    // Setup a wait for ready command to specified cs.
    ctrl0.U = (BF_GPMI_CTRL0_CS(cs) |
               BV_FLD(GPMI_CTRL0, LOCK_CS, DISABLED) |
               BF_GPMI_CTRL0_XFER_COUNT(0) |
               BF_GPMI_CTRL0_ADDRESS(0) |
               BV_FLD(GPMI_CTRL0, ADDRESS_INCREMENT, DISABLED) |
               BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT) |
               BV_FLD(GPMI_CTRL0, UDMA, DISABLED) |
               BV_FLD(GPMI_CTRL0, COMMAND_MODE, WAIT_FOR_READY) | BF_GPMI_CTRL0_RUN(1));
    HW_GPMI_CTRL0_WR(ctrl0.U);

    // Poll for cmd end to toggle.
    if (!gpmi_poll_debug(GPMI_DEBUG_CMD_END(cs),
                         ~debug.U & GPMI_DEBUG_CMD_END(cs), WAIT_FOR_READY_TIMEOUT)) {
    }
    // Poll for wait for ready to toggle.
    if (!gpmi_poll_debug(GPMI_DEBUG_WAIT_FOR_READY(cs),
                         ~debug.U & GPMI_DEBUG_WAIT_FOR_READY(cs), WAIT_FOR_READY_TIMEOUT)) {
    }
}

/**
  * hal_nand_send_cmd - send command to nand chip
  * @cs: chip select
  * @cmd: send command to nand chip
  *
  * return TRUE if succeed, otherwise return FALSE
  */
BOOL hal_nand_send_cmd(U32 cs, U8 cmd)
{
    BOOL res = FALSE;
    apbh_dma_gpmi3_t dma_seed;
    apbh_dma_t end_dma;

    dma_seed.nxt = (apbh_dma_gpmi1_t *) & end_dma;
    dma_seed.cmd.U = NAND_DMA_COMMAND_CMD(1, 0, NAND_LOCK, 3);
    dma_seed.bar = &cmd;

    dma_seed.apbh_dma_gpmi3_u.apbh_dma_gpmi3_ctrl.gpmi_ctrl0.U =
        NAND_DMA_COMMAND_PIO(cs, 1, 0, ASSERT_CS);
    dma_seed.apbh_dma_gpmi3_u.apbh_dma_gpmi3_ctrl.gpmi_compare.U = 0;
    dma_seed.apbh_dma_gpmi3_u.apbh_dma_gpmi3_ctrl.gpmi_eccctrl.U =
        NAND_DMA_ECC_PIO(BV_GPMI_ECCCTRL_ENABLE_ECC__DISABLE);

    end_dma.nxt = (apbh_dma_t *) 0x0;
    // Decrement semaphore, set IRQ, no DMA transfer.
    end_dma.cmd.U = ((reg32_t)
                     (BF_APBH_CHn_CMD_IRQONCMPLT(1) |
                      BF_APBH_CHn_CMD_SEMAPHORE(1) | BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER)));
    // BAR points to success termination code.
    end_dma.bar = (void *)0x0;

    dma_start((dma_cmd_t *) & dma_seed, cs);

    res = dma_wait(MAX_TRANSACTION_TIMEOUT, cs);

    return res;
}

/**
  * hal_nand_send_addr - send address to nand chip
  * @cs: chip select
  * @addr: 5 address cycles
  * @bytes: number of datas to write
  *
  * return TRUE if succeed, otherwise return FALSE
  */
BOOL hal_nand_send_addr(U32 cs, U8 * addr, U16 bytes)
{
    BOOL res = FALSE;
    apbh_dma_gpmi3_t dma_seed;
    apbh_dma_t end_dma;

    dma_seed.nxt = (apbh_dma_gpmi1_t *) & end_dma;
    dma_seed.cmd.U = (BF_APBH_CHn_CMD_XFER_COUNT(bytes) |
                      BF_APBH_CHn_CMD_CMDWORDS(3) |
                      BF_APBH_CHn_CMD_WAIT4ENDCMD(1) |
                      BF_APBH_CHn_CMD_SEMAPHORE(0) |
                      BF_APBH_CHn_CMD_NANDLOCK(1) |
                      BF_APBH_CHn_CMD_CHAIN(1) | BV_FLD(APBH_CHn_CMD, COMMAND, DMA_READ));

    dma_seed.bar = addr;

    dma_seed.apbh_dma_gpmi3_u.apbh_dma_gpmi3_ctrl.gpmi_ctrl0.U =
        (BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE) |
         BF_GPMI_CTRL0_WORD_LENGTH(BV_GPMI_CTRL0_WORD_LENGTH__8_BIT) | BF_GPMI_CTRL0_LOCK_CS(1) |
         BF_GPMI_CTRL0_CS(cs) | BV_FLD(GPMI_CTRL0, ADDRESS,
                                       NAND_ALE) | BF_GPMI_CTRL0_XFER_COUNT(bytes));

    dma_seed.apbh_dma_gpmi3_u.apbh_dma_gpmi3_ctrl.gpmi_compare.U = 0;
    dma_seed.apbh_dma_gpmi3_u.apbh_dma_gpmi3_ctrl.gpmi_eccctrl.U =
        NAND_DMA_ECC_PIO(BV_GPMI_ECCCTRL_ENABLE_ECC__DISABLE);

    end_dma.nxt = (apbh_dma_t *) 0x0;
    // Decrement semaphore, set IRQ, no DMA transfer.
    end_dma.cmd.U = ((reg32_t)
                     (BF_APBH_CHn_CMD_IRQONCMPLT(1) |
                      BF_APBH_CHn_CMD_SEMAPHORE(1) | BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER)));
    // BAR points to success termination code.
    end_dma.bar = (void *)0x0;

    dma_start((dma_cmd_t *) & dma_seed, cs);

    res = dma_wait(MAX_TRANSACTION_TIMEOUT, cs);

    return res;
}

/**
  * hal_nand_read_raw_data - read raw data without ECC and randomizer
  * @cs: chip select
  * @buf: buffer which contain data to write into nand
  * @bytes: number of datas to write
  * @bDDR: if TRUE, 16bit bus will be used, otherwise 8bit bus
  *
  * return TRUE if succeed, otherwise return FALSE
  */
BOOL hal_nand_read_raw_data(U32 cs, U8 * buf, U16 bytes, BOOL bDDR)
{
    BOOL res = FALSE;
    apbh_dma_gpmi3_t dma_seed;
    apbh_dma_t end_dma;
    U32 read_size = bytes;
    U32 word_len = BV_GPMI_CTRL0_WORD_LENGTH__8_BIT;

    if (bDDR) {
        read_size = bytes >> 1;
        word_len = BV_GPMI_CTRL0_WORD_LENGTH__16_BIT;
    }

    dma_seed.nxt = (apbh_dma_gpmi1_t *) & end_dma;
    dma_seed.cmd.U = (BF_APBH_CHn_CMD_XFER_COUNT(bytes) |
                      BF_APBH_CHn_CMD_CMDWORDS(1) |
                      BF_APBH_CHn_CMD_WAIT4ENDCMD(1) |
                      BF_APBH_CHn_CMD_SEMAPHORE(0) |
                      BF_APBH_CHn_CMD_NANDLOCK(0) |
                      BF_APBH_CHn_CMD_CHAIN(1) | BV_FLD(APBH_CHn_CMD, COMMAND, DMA_WRITE));
    dma_seed.bar = buf;
    dma_seed.apbh_dma_gpmi3_u.apbh_dma_gpmi3_ctrl.gpmi_ctrl0.U =
        (BV_FLD(GPMI_CTRL0, COMMAND_MODE, READ) |
         BF_GPMI_CTRL0_WORD_LENGTH(word_len) |
         BF_GPMI_CTRL0_CS(cs) |
         BF_GPMI_CTRL0_LOCK_CS(0) |
         BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA) | BF_GPMI_CTRL0_XFER_COUNT(read_size));
    dma_seed.apbh_dma_gpmi3_u.apbh_dma_gpmi3_ctrl.gpmi_compare.U = 0;
    dma_seed.apbh_dma_gpmi3_u.apbh_dma_gpmi3_ctrl.gpmi_eccctrl.U =
        NAND_DMA_ECC_PIO(BV_GPMI_ECCCTRL_ENABLE_ECC__DISABLE);

    /* end_dma */
    end_dma.nxt = (apbh_dma_t *) 0x0;
    // Decrement semaphore, set IRQ, no DMA transfer.
    end_dma.cmd.U = ((reg32_t)
                     (BF_APBH_CHn_CMD_IRQONCMPLT(1) |
                      BF_APBH_CHn_CMD_SEMAPHORE(1) | BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER)));
    // BAR points to success termination code.
    end_dma.bar = (void *)0x0;

    dma_start((dma_cmd_t *) & dma_seed, cs);

    res = dma_wait(MAX_TRANSACTION_TIMEOUT, cs);

    return res;
}

/**
  * hal_nand_read_data - read data
  * @cs: chip select
  * @buf: buffer which contain data to write into nand
  * @bytes: number of datas to write
  * @bDDR: if TRUE, 16bit bus will be used, otherwise 8bit bus
  * @ecc_enable: determin whether use ecc or not
  * @rand_enable: determin whether use randomizer or not
  * @rand_type: randomizer type, determin by fuse setting
  * @rand_page: randomizer page, used for page data randomize, related to page address
  *
  * return TRUE if succeed, otherwise return FALSE
  */
BOOL hal_nand_read_data(U32 cs, U8 * buf, U16 bytes, BOOL bDDR, BOOL ecc_enable,
                                     BOOL rand_enable, U32 rand_type, U32 rand_page)
{
    apbh_dma_gpmi6_t dma_seed;
    apbh_dma_t end_dma;
    apbh_dma_gpmi3_t wait4done_dma;
	bchcfg_t bchcfg;
    U32 data_size = bytes;
    U32 word_len = BV_GPMI_CTRL0_WORD_LENGTH__8_BIT;

    if (bDDR) {
        data_size = bytes >> 1;
        word_len = BV_GPMI_CTRL0_WORD_LENGTH__16_BIT;
    }

    /*dma_seed */
	dma_seed.nxt = (apbh_dma_gpmi1_t *) & wait4done_dma;

    if (ecc_enable) {
        /*
		   Configure APBH DMA to NOT read any bytes from the NAND into
           memory using GPMI.  The ECC will become the Bus Master and 
           write the read data into memory.
           Wait for end command from GPMI before next part of chain.
           Lock GPMI to this NAND during transfer.
           NO_DMA_XFER - No DMA transfer occurs on APBH - see above.
           Decrement Semaphore to indicate finished.
		*/
        dma_seed.cmd.U = NAND_DMA_RX_CMD_ECC(0, 0);
        dma_seed.bar = 0x00; // This field isn't used.
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_compare.U = 0x00; // This field isn't used.
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_eccctrl.U =
            NAND_DMA_ECC_CTRL_PIO(BV_GPMI_ECCCTRL_BUFFER_MASK__BCH_PAGE, 0);
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_ecccount.B.COUNT = data_size;
    } else {
        /*
		 * ECC is disabled. Configure DMA to write directly to memory.
         * Wait for end command from GPMI before next part of chain.
         * Lock GPMI to this NAND during transfer.
		 */
        dma_seed.cmd.U = NAND_DMA_RX_NO_ECC_CMD(bytes, 0);
        dma_seed.bar = (void *)(((U32) buf) & 0xFFFFFFFC);    // not sure if this is right...
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_compare.U = 0x00; // This field isn't used.
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_eccctrl.U = 0;    // This field isn't used.
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_ecccount.U = 0;   // This field isn't used.
    }

    if (rand_enable) {
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_eccctrl.U |=
            BF_GPMI_ECCCTRL_RAND_ENABLE(BV_GPMI_ECCCTRL_RAND_ENABLE__ENABLE);
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_eccctrl.U |=
            BF_GPMI_ECCCTRL_RAND_TYPE(rand_type);
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_ecccount.U &=
            ~BM_GPMI_ECCCOUNT_RAND_PAGE;
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_ecccount.U |=
            BF_GPMI_ECCCOUNT_RAND_PAGE(rand_page);

        /* 
		 * If HW ECC is disable, enable it since its in datapath but bypass ECC by setting level to NONE 
		 * The flash layout should be configured even ECC level is 0. 

		 * To be simple, set the blockN size as 0x200, and block0 size as 0x200 + physical_page_size%0x200,
		 * will be less than 0x400 which is the max block size for GF13.
		 * Set metasize as 0, since there should be no metadata for raw data, such as FCB.
		 *
		 * Since metasize is 0, the page size should be 4 bytes alligned, which was usually can be met.
		 */
        if (!ecc_enable) {
            dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_eccctrl.U |=
                BV_FLD(GPMI_ECCCTRL, ENABLE_ECC, ENABLE) | BM_GPMI_ECCCTRL_BUFFER_MASK;
            dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_ecccount.U |= data_size;

			bchcfg.bch_type = 0;
			bchcfg.blockn_num_per_page = bytes / DUMMY_BLOCKN_SIZE - 1;
			bchcfg.meta_sz = 0;
			bchcfg.block0_ecc_level = 0; 
			bchcfg.block0_sz = (DUMMY_BLOCKN_SIZE + bytes % DUMMY_BLOCKN_SIZE);
			bchcfg.total_page_sz = bytes;
			bchcfg.blockn_ecc_level = 0;
			bchcfg.blockn_sz = DUMMY_BLOCKN_SIZE;

			if (!bch_set_flash_layout(cs, &bchcfg))
				return FALSE;
        }

        dma_seed.bar = 0x00; // This field isn't used, clear it per IP owner's suggestion.
        dma_seed.cmd.U = NAND_DMA_RX_CMD_ECC(0, 0); // 6 PIOs should be issued no matter EnableHWECC set or not
    }

    dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_payload.U = (((U32) buf) & 0xFFFFFFFC);
    dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_auxiliary.U = (((U32)(buf + NAND_AUX_BUFFER_INDEX)) & 0xFFFFFFFC);
    dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_ctrl0.U = NAND_DMA_RX_PIO(cs, word_len, data_size);

	/* wait for done, disable chip select and ECC block */
    wait4done_dma.nxt = (apbh_dma_gpmi1_t *) & end_dma;
    wait4done_dma.cmd.U = NAND_DMA_DISABLE_ECC_TRANSFER;  // Configure to send 3 GPMI PIO reads.
    wait4done_dma.bar = 0;
    wait4done_dma.apbh_dma_gpmi3_u.apbh_dma_gpmi3_ctrl.gpmi_ctrl0.U =
        NAND_DMA_DISABLE_ECC_PIO(cs);	// Disable the Chip Select and other outstanding GPMI things
    wait4done_dma.apbh_dma_gpmi3_u.apbh_dma_gpmi3_ctrl.gpmi_compare.U = 0x00;
    wait4done_dma.apbh_dma_gpmi3_u.apbh_dma_gpmi3_ctrl.gpmi_eccctrl.U =
        BF_GPMI_ECCCTRL_ENABLE_ECC(BV_GPMI_ECCCTRL_ENABLE_ECC__DISABLE);	// Disable the ECC Block.

    /* end_dma */
    end_dma.nxt = (apbh_dma_t *) 0x0;
    // Decrement semaphore, set IRQ, no DMA transfer.
    end_dma.cmd.U = ((reg32_t) (BF_APBH_CHn_CMD_IRQONCMPLT(1)
                                | BF_APBH_CHn_CMD_WAIT4ENDCMD(0)
                                | BF_APBH_CHn_CMD_SEMAPHORE(1)
                                | BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER)));
    end_dma.bar = (void *)0x0;

	/* start dma and wait for it */
    dma_start((dma_cmd_t *) & dma_seed, cs);
    if (!dma_wait(MAX_TRANSACTION_TIMEOUT, cs)) {
		printf("wait dma timeout\n");
		return FALSE;
	}

    return TRUE;
}

/**
  * hal_nand_write_raw_data - write data without ecc check
  * @cs: chip select
  * @buf: buffer which contain data to write into nand
  * @bytes: number of datas to write
  * @bDDR: if TRUE, 16bit bus will be used, otherwise 8bit bus
  * @rand_enable: determin whether use randomizer or not
  * @rand_type: randomizer type, determin by fuse setting
  * @rand_page: randomizer page, used for page data randomize, related to page address
  *
  * return TRUE if succeed, otherwise return FALSE
  */
BOOL hal_nand_write_raw_data(U32 cs, U8 * buf, U16 bytes, BOOL bDDR,
                             BOOL rand_enable, U32 rand_type, U32 rand_page)
{
    BOOL res = FALSE;
    apbh_dma_gpmi6_t dma_seed;
    apbh_dma_t end_dma;
    U32 write_size = bytes;
    U32 word_len = BV_GPMI_CTRL0_WORD_LENGTH__8_BIT;
	bchcfg_t bchcfg;

    /* determin use 8bit bus or 16bit bus */
    if (bDDR) {
        write_size = bytes >> 1;
        word_len = BV_GPMI_CTRL0_WORD_LENGTH__16_BIT;
    }

    /* set nxt, cmd, bar and pios */
    dma_seed.nxt = (apbh_dma_gpmi1_t *) & end_dma;
    dma_seed.cmd.U = (BF_APBH_CHn_CMD_XFER_COUNT(bytes) |
                      BF_APBH_CHn_CMD_CMDWORDS(1) |
                      BF_APBH_CHn_CMD_WAIT4ENDCMD(1) |
                      BF_APBH_CHn_CMD_SEMAPHORE(0) |
                      BF_APBH_CHn_CMD_NANDLOCK(0) |
                      BF_APBH_CHn_CMD_CHAIN(1) | BV_FLD(APBH_CHn_CMD, COMMAND, DMA_READ));
    dma_seed.bar = buf;
    dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_ctrl0.U =
        (BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE) |
         BF_GPMI_CTRL0_WORD_LENGTH(word_len) |
         BF_GPMI_CTRL0_CS(cs) |
         BF_GPMI_CTRL0_LOCK_CS(0) |
         BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA) | BF_GPMI_CTRL0_XFER_COUNT(write_size));
    dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_compare.U = 0;
    dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_eccctrl.U =
        NAND_DMA_ECC_PIO(BV_GPMI_ECCCTRL_ENABLE_ECC__DISABLE);
    dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_ecccount.U = 0;  // This field isn't used.

    /* if randomizer used, we must set related regs, enable ecc with ecc level 0, and set cmd, and bar */
    if (rand_enable) {

        /* set randomizer related regs: gpmi_eccctrl, gpmi_ecccount */
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_eccctrl.U |=
            BF_GPMI_ECCCTRL_RAND_ENABLE(BV_GPMI_ECCCTRL_RAND_ENABLE__ENABLE);
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_eccctrl.U |=
            BF_GPMI_ECCCTRL_RAND_TYPE(rand_type);
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_ecccount.U &=
            ~BM_GPMI_ECCCOUNT_RAND_PAGE;
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_ecccount.U |=
            BF_GPMI_ECCCOUNT_RAND_PAGE(rand_page);

        /* set cmd, bar and pios to enable ecc, we set ecc level as 0 */
        dma_seed.cmd.U = (BF_APBH_CHn_CMD_XFER_COUNT(0) |
                          BF_APBH_CHn_CMD_CMDWORDS(6) |
                          BF_APBH_CHn_CMD_WAIT4ENDCMD(1) |
                          BF_APBH_CHn_CMD_SEMAPHORE(0) |
                          BF_APBH_CHn_CMD_NANDLOCK(1) |
                          BF_APBH_CHn_CMD_CHAIN(1) | BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER));
        dma_seed.bar = 0x00;    // This field isn't used, clear it per IP owner's suggestion.
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_ctrl0.U =
            (BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE) |
             BF_GPMI_CTRL0_WORD_LENGTH(word_len) |
             BF_GPMI_CTRL0_CS(cs) |
             BF_GPMI_CTRL0_LOCK_CS(1) |
             BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA) | BF_GPMI_CTRL0_XFER_COUNT(0));
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_eccctrl.U |=
            NAND_DMA_ECC_CTRL_PIO(BV_GPMI_ECCCTRL_BUFFER_MASK__BCH_PAGE, 1);
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_ecccount.U |= write_size;
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_payload.U = (((U32) buf) & 0xFFFFFFFC);
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_auxiliary.U =
            (((U32) (nand.aux_buffer)) & 0xFFFFFFFC);

        /* set flash layout for bch 
         * To be simple, set the blockN size as 0x200, and block0 size as 0x200 + physical_page_size%0x200, 
         * will be less than 0x400 which is the max block size for GF13. 
         * Set metasize as 0, since there should be no metadata for raw data, such as FCB.
         *
         * Since metasize is 0, the page size should be 4 bytes alligned, which was usually can be met.
         */
        memset(&bchcfg, 0, sizeof(bchcfg));
        bchcfg.block0_ecc_level = 0;
        bchcfg.blockn_ecc_level = 0;
        bchcfg.bch_type = 0;
        bchcfg.blockn_num_per_page = write_size / DUMMY_BLOCKN_SIZE - 1;
        bchcfg.meta_sz = 0;
        bchcfg.block0_sz = (DUMMY_BLOCKN_SIZE + write_size % DUMMY_BLOCKN_SIZE);
        bchcfg.total_page_sz = write_size;
        bchcfg.blockn_sz = DUMMY_BLOCKN_SIZE;

        bch_set_flash_layout(0, &bchcfg);
    }

    /* end_dma */
    end_dma.nxt = (apbh_dma_t *) 0x0;
    // Decrement semaphore, set IRQ, no DMA transfer.
    end_dma.cmd.U = ((reg32_t)
                     (BF_APBH_CHn_CMD_IRQONCMPLT(1) |
                      BF_APBH_CHn_CMD_SEMAPHORE(1) | BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER)));
    // BAR points to success termination code.
    end_dma.bar = (void *)0x0;

    dma_start((dma_cmd_t *) & dma_seed, cs);

    res = dma_wait(MAX_TRANSACTION_TIMEOUT, cs);

    return res;
}

/**
  * hal_nand_write_data_with_ecc - write data with ecc check
  * @cs: chip select
  * @buf: buffer which contain data to write into nand
  * @bytes: number of datas to write
  * @bDDR: if TRUE, 16bit bus will be used, otherwise 8bit bus
  * @rand_enable: determin whether use randomizer or not
  * @rand_type: randomizer type, determin by fuse setting
  * @rand_page: randomizer page, used for page data randomize, related to page address
  *
  * return TRUE if succeed, otherwise return FALSE
  */
BOOL hal_nand_write_data_with_ecc(U32 cs, U8 * buf, U16 bytes, BOOL bDDR,
                              BOOL rand_enable, U32 rand_type, U32 rand_page)
{
    BOOL retCode = FALSE;
    apbh_dma_gpmi6_t dma_seed;
    apbh_dma_t end_dma;
    U32 u32WriteDataSize = bytes;
    U32 word_len = BV_GPMI_CTRL0_WORD_LENGTH__8_BIT;

    if (bDDR) {
        u32WriteDataSize = bytes >> 1;
        word_len = BV_GPMI_CTRL0_WORD_LENGTH__16_BIT;
    }

    /* dma_seed */
    dma_seed.nxt = (apbh_dma_gpmi1_t *) & end_dma;
    dma_seed.cmd.U = (BF_APBH_CHn_CMD_XFER_COUNT(0) |
                      BF_APBH_CHn_CMD_CMDWORDS(6) |
                      BF_APBH_CHn_CMD_WAIT4ENDCMD(1) |
                      BF_APBH_CHn_CMD_SEMAPHORE(0) |
                      BF_APBH_CHn_CMD_NANDLOCK(1) |
                      BF_APBH_CHn_CMD_CHAIN(1) | BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER));
    dma_seed.bar = 0x00;
    dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_ctrl0.U =
        (BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE) |
         BF_GPMI_CTRL0_WORD_LENGTH(word_len) |
         BF_GPMI_CTRL0_CS(cs) |
         BF_GPMI_CTRL0_LOCK_CS(1) |
         BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA) | BF_GPMI_CTRL0_XFER_COUNT(0));
    dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_compare.U = 0;
    dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_eccctrl.U =
        NAND_DMA_ECC_CTRL_PIO(BV_GPMI_ECCCTRL_BUFFER_MASK__BCH_PAGE, 1);
    dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_ecccount.B.COUNT = u32WriteDataSize;
    dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_payload.U = (((U32) buf) & 0xFFFFFFFC);
    dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_auxiliary.U =
        (((U32) (nand.aux_buffer)) & 0xFFFFFFFC);

    if (rand_enable) {
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_eccctrl.U |=
            BF_GPMI_ECCCTRL_RAND_ENABLE(BV_GPMI_ECCCTRL_RAND_ENABLE__ENABLE);
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_eccctrl.U |=
            BF_GPMI_ECCCTRL_RAND_TYPE(rand_type);
        /* Clear the randomizer page and the set it */
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_ecccount.U &=
            ~BM_GPMI_ECCCOUNT_RAND_PAGE;
        dma_seed.apbh_dma_gpmi6_u.apbh_dma_gpmi6_ctrl.gpmi_ecccount.U |=
            BF_GPMI_ECCCOUNT_RAND_PAGE(rand_page);
    }

    /* end_dma */
    end_dma.nxt = (apbh_dma_t *) 0x0;
    // Decrement semaphore, set IRQ, no DMA transfer.
    end_dma.cmd.U = ((reg32_t)
                     (BF_APBH_CHn_CMD_IRQONCMPLT(1) |
                      BF_APBH_CHn_CMD_SEMAPHORE(1) | BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER)));
    // BAR points to success termination code.
    end_dma.bar = (void *)0x0;
    dma_start((dma_cmd_t *) & dma_seed, cs);

    retCode = dma_wait(MAX_TRANSACTION_TIMEOUT, cs);

    return retCode;
}

