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
#define CMD_ADDR0			(0x555)
#define CMD_ADDR1			(0x2AA)
#define CMD_ADDR2			(0xAAA)
#define CMD_ADDR_READ_CFI	(0x55)

#define CMD_READ_ID			(0x90)
#define CMD_RESET			(0xF0)
#define CMD_UNLOCK_1		(0xAA)
#define CMD_UNLOCK_2		(0x55)
#define CMD_READ_CFI		(0x98)

#define ADDR_MANU_ID		(0x0000)
#define ADDR_DEVICE_ID_1	(0x0001)
#define ADDR_DEVICE_ID_2	(0x000E)
#define ADDR_DEVICE_ID_3	(0x000F)

/*===================================================================================
                                        GLOBAL VARIABLES
==================================================================================*/
extern nor_cfg_t *g_nor_cfg;

/*===================================================================================
                                        GLOBAL FUNCTIONS
==================================================================================*/
/**
  * nor_reset - reset nor, will enter read mode
  *
  * return true if succeed, otherwise return false
  */
static void nor_reset(void)
{
	send_cmd_w((U16)CMD_RESET, CMD_ADDR0);
}

/**
  * nor_unlock - unlock nor
  * @block_addr: useless in nor which supports cmdset0
  *
  * return true if succeed, otherwise return false
  */
static void nor_unlock(U32 block_addr)
{
	send_cmd_w((U16)CMD_UNLOCK_1, CMD_ADDR0);
	send_cmd_w((U16)CMD_UNLOCK_2, CMD_ADDR1);
}

/**
  * nor_read_id - read nor manufacture id and device ids,
  				  and check them if correct
  *
  * return true if succeed, otherwise return false
  */
static void nor_read_id(U8 *ids)
{
	/* reset and unlock first */
    nor_reset();
    nor_unlock(0);

	/* enter auto select mode and read ids */
	send_cmd_w((U16)CMD_READ_ID, CMD_ADDR0);

	/* get id datas */
	ids[0] = get_data_w(ADDR_MANU_ID);
	ids[1] = get_data_w(ADDR_DEVICE_ID_1);
	ids[2] = get_data_w(ADDR_DEVICE_ID_2);
	ids[3] = get_data_w(ADDR_DEVICE_ID_3);
}

static BOOL nor_wait_for_ready(U32 addr)
{
	volatile U16 status;
	volatile U16 old_status;

	old_status = get_data_w(addr);		// read status first
	while (1) {
		status = get_data_w(addr);		// read status again
		if ((old_status & 0x40) == (status & 0x40)) {	// toggle bit
			return TRUE;
		}
		if(status & 0x20) {		// error bit, timeout
			/* read and compare again to incase complete just before timeout */
			old_status = get_data_w(addr);
			status = get_data_w(addr);
			if ((old_status & 0x40) == (status & 0x40))
				return TRUE;
			else {
				printf("wait for ready status: 0x%2x\n", status);
				return FALSE;
			}
		}
		old_status = status;
	}

	return TRUE;
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

    nor_reset();
    nor_unlock(0);

	/* send cmd */
	send_cmd_w((U16)0x80, CMD_ADDR0);
	send_cmd_w((U16)0xAA, CMD_ADDR0);
	send_cmd_w((U16)0x55, CMD_ADDR1);
	send_cmd_w((U16)0x30, addr);
	
	/* wait for ready */
	if(!nor_wait_for_ready(0)) {
		printf("nor wait for ready timeout!\n");
		return FALSE;
	}

	return TRUE;
}

/**
  * nor_erase_chip - erase full chip, typical time 80s, max 800s
  *
  * return true if succeed, otherwise return false
  */
static BOOL nor_erase_chip(void)
{
    nor_reset();
    nor_unlock(0);

	/* send cmds */
	send_cmd_w((U16)0x80, CMD_ADDR0);
	send_cmd_w((U16)0xAA, CMD_ADDR0);
	send_cmd_w((U16)0x55, CMD_ADDR1);
	send_cmd_w((U16)0x10, CMD_ADDR0);

	/* wait for ready */
	if(!nor_wait_for_ready(0)) {
		printf("nor wait for ready timeout!\n");
		return FALSE;
	}

	return TRUE;
}

/**
  * nor_read_words - read words from nor
  * @addr: start address of nor flash to read
  * @len: number of bytes to read
  * @buf: buffer to store datas read from nor
  *
  * return true if succeed, otherwise return false
  */
static BOOL nor_read_words(U32 addr, U32 len, U16 *buf)
{
	U32 leftw = (len + 1) / 2;

    nor_reset();

	addr >>= 1;		// 16bit data, word align
	while(leftw--) {
		*buf++ = get_data_w(addr++);
	}

	return TRUE;
}

/**
  * nor_write_words - write words into nor, typical 16us, max 200us
  * @addr: address to write word, must be word align
  * @buf: buffer which store data to be written into nor
  * @len: length of data to be written, in Byte.
  *
  * return true if succeed, otherwise return false
  */
static BOOL nor_write_words(U32 addr, U16 *buf, U32 len)
{
    nor_reset();
    nor_unlock(0);

	/* init variables first */
	len = (len - 1) / 2 * 2 + 2;        // make len to be an even number
	len >>= 1;      					// 16bit dada, use word count
	addr >>= 1;							// 16bit data, word align

	/* write one word one cycle */
	while(len) { 
		/* reset and unlock needed for "MT28EW01GABA" */
    	nor_reset();
    	nor_unlock(0);

		/* send cmds */
		send_cmd_w((U16)0xA0, CMD_ADDR0);
		send_cmd_w((U16)(*buf), addr);

		/* wait for ready */
		if(!nor_wait_for_ready(0)) {
			printf("nor wait for ready timeout!\n");
			return FALSE;
		}

		/* set variables */
		addr++;
		buf++;
		len--;
	}

	return TRUE;
}


/* fill nor_cmdset_t struct */
nor_cmdset_t nor_cmdset0 = {
	.reset = nor_reset,
	.unlock = nor_unlock,
	.read_id = nor_read_id,
	.erase_block = nor_erase_block,
	.erase_chip = nor_erase_chip,
	.read_words = nor_read_words,
	.write_words = nor_write_words,
};


