/*==================================================================================================
    Copyright (C) 2006-2011  Freescale Semiconductor, Inc. All Rights Reserved
    THIS SOURCE CODE IS CONFIDENTIAL AND PROPRIETARY AND MAY NOT
    BE USED OR DISTRIBUTED WITHOUT THE WRITTEN PERMISSION OF
    Freescale Semiconductor, Inc.
*===================================================================================================*/

/*
Revision History:
                            Modification     Tracking
Author                          Date          Number     Description of Changes
-------------------------   ------------    ----------   -------------------------------------------
Jerry Fan                 27/Sept/2011    ENGR00151908
*/

/*===================================================================================
                                        INCLUDE FILES
==================================================================================*/
#include <io.h>
#include "nor.h"

/*===================================================================================
                                        MACROS
==================================================================================*/
#define CMD_READ_ID				(0x90)
#define CMD_BLOCK_UNLOCK1		(0x60)
#define CMD_BLOCK_UNLOCK2		(0xd0)
#define CMD_READ_ARRAY			(0xff)
#define CMD_BLOCK_ERASE1		(0x20)
#define CMD_BLOCK_ERASE2		(0xd0)
#define CMD_CLEAR_STATUS_REG	(0x50)
#define CMD_READ_STATUS_REG		(0x70)
#define CMD_BUFFER_PROGRAM1		(0xE9)
#define CMD_BUFFER_PROGRAM2		(0xD0)

#define BUFFER_PROGRAM_SIZE		(512)
/*===================================================================================
                                        GLOBAL VARIABLES
==================================================================================*/
extern nor_cfg_t *g_nor_cfg;

/*===================================================================================
                                        GLOBAL FUNCTIONS
==================================================================================*/
/**
  * nor_reset - reset muxnor, will enter read mode
  *
  * return true if succeed, otherwise return false
  */
static void nor_reset(void)
{
	send_cmd_w((U16)0xff, 0);
}

/**
  * nor_unlock - unlock muxnor
  * @block_addr: block address to unlock
  *
  * return true if succeed, otherwise return false
  */
static void nor_unlock(U32 block_addr)
{
	U32 addr = (block_addr * g_nor_cfg->block_size) >> 1;	// 16bit data, must be aligned

	send_cmd_w(CMD_BLOCK_UNLOCK1, addr);
	send_cmd_w(CMD_BLOCK_UNLOCK2, addr);
}

/**
  * nor_read_id - read muxnor manufacture id and device ids,
  				  and check them if correct
  *
  * return true if succeed, otherwise return false
  */
static void nor_read_id(U8 *ids)
{
	/* enter auto select mode and read ids */
	send_cmd_w(CMD_READ_ID, 0x0);

	ids[0] = get_data_w(0);
	ids[1] = get_data_w(0x1);

}


static BOOL nor_wait_for_ready(U32 addr)
{
	volatile U16 status;

	/* wait until SR7=1, means nor flash ready */
	status = 0;
	while((status & (1 << 7)) == 0) {
		send_cmd_w(CMD_READ_STATUS_REG, 0);
		status = get_data_w(0);
	}

	/* any error occured, return FALSE */
	if ((status & 0x7f) == 0) {
		return TRUE;
	} else {
		printf("status = %d\n", status);
		
		/* clear status register */
		send_cmd_w(CMD_CLEAR_STATUS_REG, 0);
	}

	/* status check, maybe multi errors */
	if ((status & (1 << 6)) != 0) {
		printf("Erase suspend in effect\n");
	}
	if ((status & (3 << 4)) != 0) {
		switch((status & (3 << 4)) >> 4) {
		case 1:
			printf("program error, operation abored\n");
			break;
		case 2:
			printf("erase error, operation abored\n");
			break;
		case 3:
			printf("command sequence error, command abored\n"); 
			break;
		}
	}
	if ((status & (1 << 3)) != 0) {
		printf("Vpp < Vpplk during program or erase, operation aborted\n");
	}
	if ((status & (1 << 2)) != 0) {
		printf("program suspend in effect\n");
	}
	if ((status & (1 << 1)) != 0) {
		printf("block locked during program or erase, operation aborted\n");
	}

	return FALSE;
}

/**
  * nor_erase_block - erase one block, typical time 0.5s, max 2.3s
  * @block_addr: block address
  *
  * return true if succeed, otherwise return false
  */
static BOOL nor_erase_block(U32 block_addr)
{
	/* calculate address first, the first address of block to be erase */
	U32 addr = (block_addr * g_nor_cfg->block_size) >> 1;

    nor_unlock(block_addr);

	/* send cmd */
	send_cmd_w(CMD_BLOCK_ERASE1, addr);
	send_cmd_w(CMD_BLOCK_ERASE2, addr);
	
	/* wait for ready */
	if(!nor_wait_for_ready(0)) {
		printf("nor wait for ready timeout!\n");
		return FALSE;
	}

	return TRUE;
}

/**
  * nor_read_words - read words from muxnor
  * @addr: start address of muxnor flash to read
  * @len: number of bytes to read
  * @buf: buffer to store datas read from muxnor
  *
  * return true if succeed, otherwise return false
  */
static BOOL nor_read_words(U32 addr, U32 len, U16 *buf)
{
	U32 leftw = (len + 1) / 2;

	addr >>= 1;		// 16bit data, word align
	send_cmd_w(CMD_READ_ARRAY, addr);

	while(leftw--) {
		*buf++ = get_data_w(addr++);
	}

	return TRUE;
}

/**
  * nor_write_word - write one word into muxnor, typical 16us, max 200us
  * @addr: address to write word, must be word align
  * @buf: buffer which store data to be written into nor
  * @len: length of data to be written, in Byte
  *
  * return true if succeed, otherwise return false
  */
static BOOL nor_write_words(U32 addr, U16 *buf, U32 len)
{
	U32 len_step;
	U32 i;
	U32 block_addr = (addr / g_nor_cfg->block_size);

	/* unlock first, don't forget to erase block first */
    nor_unlock(block_addr);

	/* init variables */
	len = (len - 1) / 2 * 2 + 2;		// make len to be an even number
	len >>= 1;							// 16bit dada, use word count
	addr >>= 1;							// 16bit data, word align

	/* write nor buffer_size word one cycle */
	while(len) {
		len_step = ((len < BUFFER_PROGRAM_SIZE) ? len : BUFFER_PROGRAM_SIZE);

		/* send cmds */
		send_cmd_w(CMD_BUFFER_PROGRAM1, addr);		// program setup

		send_cmd_w(len_step - 1, addr);		// program load 1, word count-1

		for(i = 0; i < len_step; i++)		// program load 2, data load into nor buffer
			send_cmd_w(*(buf + i), addr + i);

		send_cmd_w(CMD_BUFFER_PROGRAM2, addr);		// program confirm

		/* wait for ready */
		if(!nor_wait_for_ready(0)) {
			printf("nor wait for ready timeout!\n");
			return FALSE;
		}

		/* set variables for next cycle */
		addr += len_step;
		buf += len_step;
		len -= len_step;
	}

	return TRUE;
}


/* fill nor_cmdset_t struct */
nor_cmdset_t nor_cmdset1 = {
	.reset = nor_reset,
	.unlock = nor_unlock,
	.read_id = nor_read_id,
	.erase_block = nor_erase_block,
	.erase_chip = NULL,
	.read_words = nor_read_words,
	.write_words = nor_write_words,
};

