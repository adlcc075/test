#ifndef ROM_NAND_FCB_DBBT_BURN_H    /* To prevent inadvertently including a header twice */
#define ROM_NAND_FCB_DBBT_BURN_H

/*==================================================================================================
     Header Name: rom_nand_burn.h
     General Description: Provide some defines for nand burn drivers
====================================================================================================

                         Copyright: 2004-2014 FREESCALE, INC.
                   All Rights Reserved. This file contains copyrighted material.
                   Use of this file is restricted by the provisions of a
                   Freescale Software License Agreement, which has either
                   accompanied the delivery of this software in shrink wrap
                   form or been expressly executed between the parties.

Revision History:
                  Modification    Tracking
Author (core ID)      Date         Number     Description of Changes
-----------------  -----------   ----------   ------------------------------------------
Terry Xie           2/Jan/2014                Initial Implementation
-----------------  ------------  ----------   ------------------------------------------
====================================================================================================
====================================================================================================
                                        INCLUDE FILES
==================================================================================*/
#include "rom_nand_boot.h"
/*================================================================================
                          Hash Defines And Macros
=================================================================================*/
#define NAND_READ_PARAMETER_PAGE_CMD		 	(0XEC)
#define NAND_READ_PARAMETER_PAGE_ONFI_ADDR 		(0X00)
#define NAND_PARAMETER_PAGE_LEN 				(0X100)
#define NAND_PARAMETER_PAGE_DATA_BYTES_POS 		(80)
#define NAND_PARAMETER_PAGE_SPARE_BYTES_POS		(84)
#define NAND_PARAMETER_PAGE_PAGES_PER_BLOCK_POS (92)
#define NAND_PARAMETER_PAGE_DEVICE_NANE_POS		(44)
#define ONFI_DEVICE_NANE_LENGTH					(20)
#define NAND_PARAMETER_PAGE_READ_RETRY_OPTIONS_POS (180)

#define FCB_COPIES_NUM	(4)
#define DBBT_COPIES_NUM	(4)

#define ROM_DBBT_MAX_BLOCK_NUM 	(50)
#define BAD_BLOCK_NUM_MAX		(10)

#define ECC_LEVEL_MAX 		(40)

/* nand options */
#define SMART_NAND				(1 << 0)
#define SYNC_MODE					(1 << 1)
#define READ_RETRY_ENABLE			(1 << 2)

/* firmware related */
#define FIRMWARE_MAX_BYTES          (1024*1024)	// used to determin the addr of firmware2 
#define FIRMWARE_INTERLEAV_BLOCKS 	(10)
#define IVT_OFFSET					(0x400)
#define FIRMWARE_LENGTH_OFFSET		(0x24)
#define FIRMWARE_FIRST 				(0)
#define FIRMWARE_SECOND 			(1)

/*==================================================================================================
                                             STRUCTURES, ENUMS, AND TYPEDEFS
==================================================================================================*/
typedef struct {
    struct {
        U32 Reserved1;
        U32 FingerPrint;
        U32 Version;
        U32 Reserved2;
        U32 DBBTNumOfPages;
    } head;
    struct {
        U32 Reserved1;
        U32 NumOfEntries;
        U32 bad_block_number[BAD_BLOCK_NUM_MAX];
    } entry;
} dbbt_block_t;

typedef struct {
    U8 ids[6];
    U32 page_size;
    U32 spare_size;
    U32 block_pages;
    U32 option;
} rom_nand_cfg_t;

/*=================================================================================
                                 Global FUNCTION PROTOTYPES
=================================================================================*/
BOOL nand_fcb_fill_page_size(U8 * ids);
void nand_fcb_dump(FCBStruct_t * pfcb);
BOOL rom_nand_fcb_burn(U8 * page_buf);
BOOL rom_nand_dbbt_burn(U8 * page_buf);
BOOL rom_nand_firmware_burn(BOOL is_skip, U8 * page_buf);
BOOL rom_nand_fcb_test_burn(U8 * page_buf, BOOL is_disbbm, int ecc_level, int toggle_speed,
							BOOL is_fcb1_error, int sync_speed, BOOL is_rand_en);

#endif /* ROM_NAND_FCB_DBBT_BURN_H */
