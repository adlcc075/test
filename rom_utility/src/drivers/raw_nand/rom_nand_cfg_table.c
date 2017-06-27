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
Terry Xie                 18/11/2014    ENGR00151908
*/

/*===================================================================================
                                        INCLUDE FILES
==================================================================================*/
#include <io.h>
#include "rom_nand_burn.h"
#include "nand_base.h"

/* nand configuration table, maybe some kinds of nand can't be found here */
const rom_nand_cfg_t rom_nand_cfg_table[] = {
    /* H27UBG8T2CTR */
    {
		.ids = { NAND_MFR_HYNIX, 0xd7, 0x94, 0x91, 0x60, 0x44 },
		.page_size = 8192,
		.spare_size = 640,
		.block_pages = 256,
		.option = READ_RETRY_ENABLE,
	},
    /* H27UCG8T2ATR */
    {
		.ids = { NAND_MFR_HYNIX, 0xde, 0x94, 0xda, 0x74, 0xc4 },
		.page_size = 8192,
		.spare_size = 640,
		.block_pages = 256,
		.option = READ_RETRY_ENABLE,
	},
    /* H27UCG8T2B*/
    {
		.ids = { NAND_MFR_HYNIX, 0xde, 0x94, 0xeb, 0x74, 0x44 },
		.page_size = 16384,
		.spare_size = 1280,
		.block_pages = 256,
		.option = READ_RETRY_ENABLE,
	},
    /* HY27UG088G5B*/
    {
		.ids = {NAND_MFR_HYNIX, 0xdc, 0x10, 0x95, 0x54, 0xad},
		.page_size = 2048,
		.spare_size = 64,
		.block_pages = 64,
	},
#if 0
    /* H27UBG8T2BTR*/
    {
		.ids = { NAND_MFR_HYNIX, 0xd7, 0x94, 0xda, 0x74, 0xc3 },
		.page_size = 8192,
		.spare_size = 640,
		.block_pages = 256,
	},
    /* H27UAG8T2A-2G*/
    {
		.ids = { NAND_MFR_HYNIX, 0xd5, 0x94, 0x25, 0x44, 0x41 },
		.page_size = 4096,
		.spare_size = 224,
		.block_pages = 128,
	},
    /* H27UCG8T2M-8G*/
    {
		.ids = { NAND_MFR_HYNIX, 0xde, 0x94, 0xd2, 0x04, 0x43 },
		.page_size = 8192,
		.spare_size = 448,
		.block_pages = 256,
	},
    /* H27UBG8T2A-4G*/
    {
		.ids = { NAND_MFR_HYNIX, 0xd7, 0x94, 0x9a, 0x74, 0x42 },
		.page_size = 8192,
		.spare_size = 448,
		.block_pages = 256,
	},
#endif
    /* SDTNRGAMA-008G, A19nm MLC */
    {
		.ids = {NAND_MFR_SANDISK, 0xde, 0x94, 0x93, 0x76, 0x50},
		.page_size = 16384,
		.spare_size = 1280,
		.block_pages = 256,
		.option = READ_RETRY_ENABLE,
	},
    /* SDTNQGAMA-008G, 19nm MLC */
    {
		.ids = {NAND_MFR_SANDISK, 0xDE, 0x94, 0x93, 0x76, 0x57},
		.page_size = 16384,
		.spare_size = 1280,
		.block_pages = 256,
		.option = READ_RETRY_ENABLE,
	},

    /* S34ML02G2, 4bit ECC, SLC, */
    {
		.ids = {NAND_MFR_SPANSION, 0xf1, 0x80, 0x1d, 0x01, 0xf1},
		.page_size = 2048,
		.spare_size = 64,
		.block_pages = 64,
	},

    /* TC58TEG6DDK, A19nm MLC */
    {
		.ids = {NAND_MFR_TOSHIBA, 0xde, 0x94, 0x93, 0x76, 0x50},
		.page_size = 16384,
		.spare_size = 1280,
		.block_pages = 256,
		.option = READ_RETRY_ENABLE,
	},
    /* TC58TEG6D2H */
    {
		.ids = {NAND_MFR_TOSHIBA, 0xd7, 0x84, 0x93, 0x72, 0x57},
		.page_size = 16384,
		.spare_size = 1280,
		.block_pages = 256,
		.option = READ_RETRY_ENABLE,
	},
    /* TC58NVG2S0F */
    {
		.ids = {NAND_MFR_TOSHIBA, 0xdc, 0x90, 0x26, 0x76, 0x15},
		.page_size = 4096,
		.spare_size = 224,
		.block_pages = 64,
	},
    /* TC58TEG6DDL,  */
    {
		.ids = {NAND_MFR_TOSHIBA, 0xde, 0x94, 0x93, 0x76, 0x51},
		.page_size = 4096,
		.spare_size = 224,
		.block_pages = 64,
	},
    /* TC58NVG5D2H */
    {
		.ids = {NAND_MFR_TOSHIBA, 0xd7, 0x94, 0x32, 0x76, 0x56},
		.page_size = 8192,
		.spare_size = 640,
		.block_pages = 256,
	},
    /* THGBR2G5D1J, smartnand */
    {
		.ids = { NAND_MFR_TOSHIBA, 0xd7, 0x84, 0x93, 0xf0, 0x57 },
		.page_size = 16384,
		.spare_size = 64,
		.block_pages = 256,
		.option = SMART_NAND,
	},
#if 0	// because of ROM size limitation, comment below nand configure, as I don't have the chips in hand
    /* TH58TEG8DDK */
    {
		.ids = { NAND_MFR_TOSHIBA, 0x3a, 0x95, 0x93, 0x7a, 0x50 },
		.page_size = 16384,
		.spare_size = 1280,
		.block_pages = 256,
	},
    /* TC58NVG6D2F-8G */
    {
		.ids = { NAND_MFR_TOSHIBA, 0xde, 0x94, 0x32, 0x76, 0x55 },
		.page_size = 8192,
		.spare_size = 448,
		.block_pages = 128,
	},
    /* TH58TVG7D2GBA */
    {
		.ids = { NAND_MFR_TOSHIBA, 0xde, 0x94, 0x82, 0x76, 0xd6 },
		.page_size = 8192,
		.spare_size = 640,
		.block_pages = 256,
	},
    /* K9GAG08U0M-2G */
    {
		.ids = {NAND_MFR_SAMSUNG, 0xd5, 0x14, 0xb6, 0x74, 0x00},
		.page_size = 4096,
		.spare_size = 128,
		.block_pages = 128,
	},
    /* K9GAG08U0E-2G */
    {
		.ids = {NAND_MFR_SAMSUNG, 0xd5, 0x84, 0x72, 0x50, 0x42},
		.page_size = 8192,
		.spare_size = 436,
		.block_pages = 128,
	},
    /* K9GBG08U0A-4G */
    {
		.ids = {NAND_MFR_SAMSUNG, 0xd7, 0x94, 0x7a, 0x54, 0x43},
		.page_size = 8192,
		.spare_size = 640,
		.block_pages = 128,
	},
    /* K9LCG08U0A-8G */
    {
		.ids = {NAND_MFR_SAMSUNG, 0xde, 0xd5, 0x7a, 0x58, 0x43},
		.page_size = 8192,
		.spare_size = 640,
		.block_pages = 128,
	},
    /* K9ACGD8U0M-8G */
    {
		.ids = {NAND_MFR_SAMSUNG, 0xde, 0x98, 0xce, 0x74, 0xc3},
		.page_size = 8192,
		.spare_size = 1024,
		.block_pages = 192,
	},
    /* K9GBG08U0B-4G */
    {
		.ids = {NAND_MFR_SAMSUNG, 0xD7, 0x94, 0x7E, 0x64, 0x44},
		.page_size = 8192,
		.spare_size = 1024,
		.block_pages = 128,
	},
    /* K9LCG08U0B-8G */
    {
		.ids = {NAND_MFR_SAMSUNG, 0xDE, 0xD5, 0x7E, 0x68, 0x44},
		.page_size = 8192,
		.spare_size = 1024,
		.block_pages = 128,
	},
#endif
};

U32 nand_cnt_in_table = ARRAY_SIZE(rom_nand_cfg_table);
