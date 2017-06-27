/*=================================================================================

    Module Name:  pu_irom_nand_interface.c

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

Portability: Portable to other compilers or platforms.

===================================================================================
                                        INCLUDE FILES
==================================================================================*/
#include  "io.h"
#include "hamming.h"
#include "hal_bch.h"
#include "hal_nand.h"
#include "nand_read_retry.h"
#include "nand_base.h"

/*===================================================================================
                                        GLOBAL VARIABLES
==================================================================================*/
nand_t nand;
extern uint32_t regs_GPMI_base;
extern uint32_t regs_BCH_base;

/*===================================================================================
                                        LOCAL FUNCTIONS
==================================================================================*/
static void nand_swap_bbm(U8 * data_buf, U8 * metadata_buf);

/*===================================================================================
                                        Functions
==================================================================================*/
/*==========================================================================
FUNCTION: U32 interface_nand_reset(nand_info_t *nand_conf)
DESCRIPTION:
   interface_nand_reset() function will reset the NFC interface and nand flash.

ARGUMENTS PASSED:
   nand_info_t *nand_conf - NAND configuration data selected by fuses

RETURN VALUE:
   U32 - SUCCESS (TRUE) or  FAIL

PRE-CONDITIONS:
   None

POST-CONDITIONS:
   None

Detailed Description:

==============================================================================*/
BOOL nand_reset(U32 cs)
{
    if (!hal_nand_send_cmd(cs, NAND_RESET_CLE))
        return FALSE;

    hal_nand_wait_for_ready(cs);

    return TRUE;
}

/**
  * nand_swap_bbm - read the data stored in metadata back into page buffer
  * @data_buf: page data buffer
  * @metadata_buf: metadata buffer
  */
static void nand_swap_bbm(U8 * data_buf, U8 * metadata_buf)
{
    U32 byte = nand.info.bbm_byte_off_in_data;
    U32 bit = nand.info.bbm_bit_off;
    U32 spare_off = nand.info.bbm_spare_offset;

    /* bad block reserve for swaping 1 bytes   */
    *(data_buf + byte) =
        (*(metadata_buf + spare_off) << bit) | (*(data_buf + byte) & (0xFF >> (8 - bit)));
    *(data_buf + 1 + byte) =
        (*(metadata_buf + spare_off) >> (8 - bit)) | (*(data_buf + byte + 1) & (0xFF << bit));
}

/**
 * nand_read_page_no_retry - read nand page, we will choose ECC or hamming for check
   without read retry feature.
 * @cs: nand chip select
 * @page_addr: nand page number to be read
 * @page_buf: buffer used when read nand page
 *
 * return true if succeed, otherwise return false
 */
BOOL nand_read_page_no_retry(U32 cs, U32 page_addr, U8 * page_buf)
{
    int i;
    U16 bytes;
    U8 addr[5];
    U8 *pFCB;
    U8 *pParity;
    U8 *metadata_buf = ((U8 *) (page_buf + NAND_AUX_BUFFER_INDEX));;

    /* Initialize ecc and randomizer settings */
	BOOL ecc_enable = (BOOL)(nand.info.ecc_en);
    BOOL rand_enable = (BOOL) (nand.info.rand_en);
    U32 rand_type = nand.info.rand_type;
    U32 rand_page = (rand_type == 1) ? (page_addr % 192) : (page_addr % 256);

    /* calculate column address and row address */
    addr[0] = 0;
    addr[1] = 0;
    for (i = 0; i < 3; i++)
        addr[i + 2] = ((page_addr >> (8 * i)) & 0xFF);

    /* number of bytes to be written into nand flash, spare data included */
	bytes = nand.info.total_page_sz;

    /* prepare for nand write, such as wait dma, wait for nand ready */
    dma_wait(MAX_TRANSACTION_TIMEOUT, cs);
    hal_nand_wait_for_ready(cs);
    bch_clear_complete_flag();
    if (ecc_enable) {
        if (!bch_set_flash_layout(cs, &(nand.info.bchcfg))) {
            return FALSE;
        }
    }

    /* send cmd1, addr, cmd2, then read data */
    hal_nand_send_cmd(cs, NAND_READ_PAGE_CLE1);
    hal_nand_send_addr(cs, addr, 5);
    hal_nand_send_cmd(cs, NAND_READ_PAGE_CLE2);
    hal_nand_wait_for_ready(cs);

	if(!hal_nand_read_data(cs, page_buf, bytes, FALSE, ecc_enable, rand_enable, rand_type, rand_page))
		return FALSE; 

    /* check ecc status to determine if this is a valid read */
    if (nand.info.ecc_en) {
        if (!bch_check_ecc_status(cs)) {
            return FALSE;
        }
    } else if (nand.info.hamming_en){
        pFCB = (U8 *) (page_buf + NAND_HC_ECC_OFFSET_DATA_COPY);
        pParity = (U8 *) (page_buf + NAND_HC_ECC_OFFSET_PARITY_COPY);

        if (!HammingCheck(pFCB, pParity))
            return FALSE;
    }

    /* read preserved data byte back into page data buffer */
    if (!(nand.info.disbbm)) {
        nand_swap_bbm(page_buf, metadata_buf);
    }

    return TRUE;
}

/**
 * nand_read_page - read page with read retry 
 * @cs: nand chip select
 * @page_addr: nand page number to be read
 * @page_buf: buffer used when read nand page
 * 
 * return true if succeed, otherwise return false
 */
BOOL nand_read_page(U32 cs, U32 page_addr, U8 * page_buf)
{
    BOOL res = FALSE;
    rr_cmd_type_e cmd_type;

    if (nand_read_page_no_retry(cs, page_addr, page_buf))
        return TRUE;
    else if (!nand.info.read_retry_en)
        return FALSE;
    else {                      // read retry start
        /* set the entry point of retry sequence by retry sequence body pointer */
        rr_seq_set_seq_entry();

        /* run read retry sequence */
        while (1) {
            if (!(rr_seq_run(cs, &cmd_type))) {
                printf("run read retry sequence error!\n");
                res = FALSE;
                break;
            } else {
                if (cmd_type == READ_PAGE) {
                    res = nand_read_page_no_retry(cs, page_addr, page_buf);
                    if (!rr_seq_run_page_read_suffix(cs)) {
                        res = FALSE;
                        break;
                    }
                    if (res)
                        rr_seq_goto_exit(cs);
                } else if (cmd_type == FINISH)  // rr sequence is FINISH, so break the while loop
                    break;
            }
        }

        /* debug result information for read retry */
        if (res)
            printf("RR success, page number is %d\n", page_addr);
        else
            printf("RR failed, page number is %d\n", page_addr);

        /* reset read retry sequence parameters when retry finished */
        rr_seq_reset_cycle_param();
    }

    return res;
}

/**
 * nand_read_multi_page - read multiple nand pages determined by the size to be read
 	We can only read continuous nand pages in one block.
	Before reading nand pages, we need to check whether this block is bad, except for FCB and DBBT reading. 
 * @page_addr: nand page start address to be read 
 * @buf: buffer used when read nand page
 * @size: the number of pages to be read 
 *
 * return true when succeed, otherwise return false
 */
BOOL nand_read_multi_page(U32 page_addr, U8 * page_buf, U32 size)
{
    while (size--) {
        if (!nand_read_page(0, page_addr, page_buf))
            return FALSE;
        page_buf += nand.info.data_page_sz;
        page_addr++;
    }

    return TRUE;
}

/**
  * nand_get_bbm - get nand bad block marker, usually bbm locates at the first byte of oob
  * @cs: chip select
  * @page_addr: page address to get bbm
  * @buf: buffer to store bbm
  *
  * return TRUE if succeed, otherwise return FALSE
  */
BOOL nand_get_bbm(U32 cs, U32 page_addr, U8 * buf)
{
    U8 addr[5];
    int i;

    /* get column addr and row addr */
    addr[0] = (nand.info.bbm_physical_off) & 0xff;
    addr[1] = ((nand.info.bbm_physical_off) >> 8) & 0xff;
    for (i = 0; i < 3; i++)
        addr[i + 2] = ((page_addr >> (8 * i)) & 0xFF);

    /* read bad block marker, not the whole page, otherwise the speed is very slow */
    if (!hal_nand_send_cmd(cs, NAND_READ_PAGE_CLE1))
        return FALSE;
    if (!hal_nand_send_addr(cs, addr, 5))
        return FALSE;
    if (!hal_nand_send_cmd(cs, NAND_READ_PAGE_CLE2))
        return FALSE;
    hal_nand_wait_for_ready(cs);
    if (!hal_nand_read_raw_data(cs, buf, 1, FALSE))
        return FALSE;

    return TRUE;
}

/**
  * is_block_bad_by_bbm: judge whether block is bad by checking bbm, using page offset read here
  * @cs: chip select
  * @block_addr: block address to test whether it is bad
  *
  * return TRUE if block is bad, otherwise return FALSE
  */
BOOL is_block_bad_by_bbm(U32 cs, U32 block_addr)
{
    U32 pages_per_block = (nand.info.pages_per_block);
    U32 page_addr;
    U32 page_addr_first = block_addr * pages_per_block;
    U32 page_addr_offset[6];
    int i;
    U8 bbm;

    page_addr_offset[0] = 0;
    page_addr_offset[1] = 1;
    page_addr_offset[2] = 2;
    page_addr_offset[3] = pages_per_block - 3;
    page_addr_offset[4] = pages_per_block - 2;
    page_addr_offset[5] = pages_per_block - 1;

    for (i = 0; i < 6; i++) {
        page_addr = page_addr_first + page_addr_offset[i];
        if ((!nand_get_bbm(0, page_addr, &bbm)) || (bbm != BAD_BLOCK_MARKER_BYTE))
            return TRUE;
    }

    return FALSE;
}

/**
  * nand_read_status: read status operation after erasing, reading, writing and other operations
                      to test whether operation is finished correct
  * @cs: chip select
  *
  * return 8 bits status
  */
U8 nand_read_status(U32 cs)
{
    U8 status;

    hal_nand_wait_for_ready(cs);
    hal_nand_send_cmd(cs, NAND_READ_STATUS_CLE);
    hal_nand_wait_for_ready(cs);
    hal_nand_read_raw_data(cs, &status, 1, FALSE);

    return status;
}

/**
  * nand_write_page - nand write page operation
  * @cs: chip select
  * @page_addr: page address to write data
  * @page_buf: buffer containing datas to write into nand
  *
  * return TRUE if succeed, otherwise return FALSE
  */
BOOL nand_write_page(U32 cs, U32 page_addr, U8 * page_buf)
{
    int i;
    U8 addr[5];
    U16 bytes;
    U8 status;

    U8 bbm;
    U32 *pMetadata = (U32 *) (((U32) (nand.aux_buffer)) & 0xFFFFFFFC); // must be word alignd
    U32 byte_offset = nand.info.bbm_byte_off_in_data;
    U32 bit_offset = nand.info.bbm_bit_off;
    U32 spareoffset = nand.info.bbm_spare_offset;

    BOOL rand_enable = (BOOL) (nand.info.rand_en);
    U32 rand_type = nand.info.rand_type;
    U32 rand_page = (rand_type == 1) ? (page_addr % 192) : (page_addr % 256);

    /* preserve bad block marker firstly when write page(in order to fit with read, we swap bbm in every page */
    if (!nand_get_bbm(cs, page_addr, &bbm)) {
        printf("get bbm failed!\n");
        return FALSE;
    }

    if (!(nand.info.disbbm)) {
        /* save read data to metadata, write bbm to bad block marker byte position to preserve bbm when writing page */
        (*(pMetadata + spareoffset)) =
            ((*(page_buf + byte_offset) >> bit_offset) |
             (*(page_buf + 1 + byte_offset) << (8 - bit_offset))) & 0xFF;
        *(page_buf + byte_offset) =
            (bbm << bit_offset) | (*(page_buf + byte_offset) & (0xFF >> (8 - bit_offset)));
        *(page_buf + 1 + byte_offset) =
            (bbm >> (8 - bit_offset)) | (*(page_buf + byte_offset + 1) & (0xFF << bit_offset));
    }

    /* calculate column address and row address */
    addr[0] = 0;
    addr[1] = 0;
    for (i = 0; i < 3; i++)
        addr[i + 2] = ((page_addr >> (8 * i)) & 0xFF);

    /* number of bytes to be written into nand flash, spare data included */
    bytes = nand.info.total_page_sz;

    /* prepare for nand write, such as wait dma, wait for nand ready */
    dma_wait(MAX_TRANSACTION_TIMEOUT, cs);
    hal_nand_wait_for_ready(cs);
    bch_clear_complete_flag();
    if (nand.info.ecc_en) {
        if (!bch_set_flash_layout(cs, &(nand.info.bchcfg)))
            return FALSE;
    }

    /* send cmd1, addr, data and cmd2 */
    hal_nand_send_cmd(cs, NAND_PROGRAM_PAGE_CLE1);
    hal_nand_send_addr(cs, addr, 5);

    if (nand.info.ecc_en) {
        if(!hal_nand_write_data_with_ecc(cs, page_buf, bytes, FALSE, rand_enable, rand_type, rand_page)) {
			printf("gpmi write data with ecc failed\n");
			return FALSE;
		}
	} else {
        if(!hal_nand_write_raw_data(cs, page_buf, bytes, FALSE, rand_enable, rand_type, rand_page)) {
			printf("hal nand write raw data failed\n");
			return FALSE;
		}
	}

    hal_nand_send_cmd(cs, NAND_PROGRAM_PAGE_CLE2);
    hal_nand_wait_for_ready(cs);

	/* check status after write */
    status = nand_read_status(0);
    if (status & NAND_PROGRAM_STATUS_FAIL) {
        printf("check status: nand write page failed!\n");
        return FALSE;
    }

    return TRUE;
}

/**
  * nand_write_page_byte: write one byte into nand page
  * @cs: chip select
  * @page_addr: page address to get bbm
  * @offset: offset in page to write byte
  * @byte: byte to write
  *
  * return TRUE if succeed, otherwise return FALSE
  */
BOOL nand_write_page_byte(U32 cs, U32 page_addr, U32 offset, U8 byte)
{
    int i;
    U8 addr[5];

    /* calculate column address and row address */
    addr[0] = (offset & 0xFF);
    addr[1] = ((offset >> 8) & 0xFF);
    for (i = 0; i < 3; i++)
        addr[i + 2] = ((page_addr >> (8 * i)) & 0xFF);

    /* send cmd, addr */
    if (!hal_nand_send_cmd(cs, NAND_PROGRAM_PAGE_CLE1))
        return FALSE;
    if (!hal_nand_send_addr(cs, addr, 5))
        return FALSE;
    if (!hal_nand_write_raw_data(cs, &byte, 1, FALSE, 0, 0, 0))
        return FALSE;
    if (!hal_nand_send_cmd(cs, NAND_PROGRAM_PAGE_CLE2))
        return FALSE;
    hal_nand_wait_for_ready(cs);

    return TRUE;
}

/**
  * nand_unlock_block - nand unlock operation, must be operated before any program and erase operation
  * @cs: chip select
  * @lower_block_addr: lower block address
  * @upper_block_addr: upper block address
  *
  * return TRUE if succeed, otherwise return FALSE
  */
BOOL nand_unlock_block(U32 cs, U32 lower_block_addr, U32 upper_block_addr)
{
	U8 status;
	U32 lower_addr = lower_block_addr * (nand.info.pages_per_block);
	U32 upper_addr = upper_block_addr * (nand.info.pages_per_block);

	/* send cmd, addr */
	hal_nand_wait_for_ready(cs);
	hal_nand_send_cmd(cs, NAND_UNLOCK_LOWER_BLOCK);
	hal_nand_send_addr(cs, (U8 *) & lower_addr, 3);
	hal_nand_send_cmd(cs, NAND_UNLOCK_UPPER_BLOCK);
	hal_nand_send_addr(cs, (U8 *) & upper_addr, 3);
	hal_nand_wait_for_ready(cs);

	/* check status */
	status = nand_read_status(0);
	if (status & NAND_ERASE_STATUS_FAIL) {
		printf("check status: nand unlock block failed!\n");
		return FALSE;
	}

	return TRUE;
}

/**
  * nand_erase_block - nand erase operation, one block as a unit
  * @cs: chip select
  * @block_addr: block address to erase
  *
  * return TRUE if succeed, otherwise return FALSE
  */
BOOL nand_erase_block(U32 cs, U32 block_addr)
{
    U8 status;
    U32 addr = block_addr * (nand.info.pages_per_block);

	/* unlock block first */
	if (!nand_unlock_block(0, block_addr, block_addr))
       	return FALSE;

	/* send cmd, addr */
    hal_nand_wait_for_ready(cs);
    hal_nand_send_cmd(cs, NAND_ERASE_BLOCK_CLE1);
    hal_nand_send_addr(cs, (U8 *) & addr, 3);
    hal_nand_send_cmd(cs, NAND_ERASE_BLOCK_CLE2);
    hal_nand_wait_for_ready(cs);

	/* check status */
    status = nand_read_status(0);
    if (!(status & NAND_WP_ENABLE)) {
        printf("check status: nand protect enabled! use unlock command\n");
        return FALSE;
    }
    else if (status & NAND_ERASE_STATUS_FAIL) {
        printf("check status: nand erase block failed!\n");
        return FALSE;
    }

    return TRUE;
}

/**
 * nand_read_id - read nand id
 * @cs: nand chip select
 * @addr: read id address, 00h or 20h
 * @ids: buffer to store ids that read from nand
 *
 * return true if succeed, otherwise return false
 */
BOOL nand_read_id(U32 cs, U8 addr, U8 * ids)
{
    if (!hal_nand_send_cmd(cs, NAND_READ_ID_CLE)) {
        return FALSE;
    }
    if (!hal_nand_send_addr(cs, &addr, 1)) {
        return FALSE;
    }

    hal_nand_wait_for_ready(cs);

    if (!hal_nand_read_raw_data(cs, ids, 6, FALSE)) {
        return FALSE;
    }

    return TRUE;
}

/**
  * nand_set_feature - nand set feature operation
  * @cs: chip select
  * @addr: feature address
  * @paras: feature parameters to write
  *
  * return TRUE if succeed, otherwise return FALSE
  */
BOOL nand_set_feature(U32 cs, U8 addr, U8 * paras)
{
    if(!hal_nand_send_cmd(cs, NAND_SET_FEATURE_CLE))
		return FALSE;

    if(!hal_nand_send_addr(cs, &addr, 1))
		return FALSE;

    if(!hal_nand_write_raw_data(cs, paras, 4, FALSE, 0, 0, 0))
		return FALSE;

    hal_nand_wait_for_ready(cs);

	return TRUE;
}

/**
  * nand_get_feature - nand get feature operation
  * @cs: chip select
  * @addr: feature address
  * @paras: feature parameters has been read 
  *
  * return TRUE if succeed, otherwise return FALSE
  */
BOOL nand_get_feature(U32 cs, U8 addr, U8 * paras)
{
    if(!hal_nand_send_cmd(cs, NAND_GET_FEATURE_CLE))
		return FALSE;

    if(!hal_nand_send_addr(cs, &addr, 1))
		return FALSE;

    hal_nand_wait_for_ready(cs);

    if(!hal_nand_read_raw_data(cs, paras, 4, FALSE))
		return FALSE;

	return TRUE;
}

/**
  * nand_mark_block_bad - mark block to be bad block by changing bbm to 0
  * @cs: chip select
  * @block_addr: block address for marking bad
  *
  * return TRUE if succeed, otherwise return FALSE
  */
BOOL nand_mark_block_bad(U32 cs, U32 block_addr)
{
    U32 page_addr = block_addr * (nand.info.pages_per_block);
    U32 offset = nand.info.bbm_physical_off;
    U8 bbm;

    /* erase nand before any write operation, don't forget! */
    if (!nand_erase_block(cs, block_addr)) {
        printf("erase block %d failed!, block is already bad\n", block_addr);
        return TRUE;
    }

	/* check whether block is already bad */
    if (is_block_bad_by_bbm(0, block_addr)) {
        printf("block %d is already bad\n", block_addr);
        return TRUE;
    }

    /* write bytes at bbm to make block bad */
    if (!nand_write_page_byte(cs, page_addr, offset, BLOCK_MARK_BAD_BYTE)) {
        printf("nand mark block %d bad failed!\n", block_addr);
        return FALSE;
    }

    /* read bbm out to confirm bbm mark success */
    if (!nand_get_bbm(cs, page_addr, &bbm)) {
        printf("get bbm failed\n");
        return FALSE;
    }
    printf("block %d bbm is %d\n", block_addr, bbm);
    if (bbm == BAD_BLOCK_MARKER_BYTE) {
        printf("bbm hasn't been changed!\n");
        return FALSE;
    }

    return TRUE;
}

