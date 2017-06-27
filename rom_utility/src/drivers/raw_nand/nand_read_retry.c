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
Terry Xie                   18/11/2014                   Transplant to rom utility
Jerry Fan                 27/Sept/2011    ENGR00151908
*/

/*===================================================================================
                                        INCLUDE FILES
==================================================================================*/
#include "io.h"
#include "nand_read_retry.h"
#include "nand_base.h"
#include "hal_nand.h"

#define NAND_READ_RETRY_DEBUG_ENABLE
#ifdef NAND_READ_RETRY_DEBUG_ENABLE
#define NAND_printf	printf
#else
#define NAND_printf
#endif

/*=========================================================================
                                      GLOBAL Variables
==========================================================================*/
/* read retry sequence table for Micron 20nm nand chips */
const U8 rr_seq_micron_20nm[] = {
    0x01, 0x00, 0x82, 0x82,     // TAG 0x82820001
    1,                          // size of reg addr array
    0x89,                       // reg addr array
    0xEF,                       // update reg cmd, used quite often, so put here
    0x4c, 0,                    // exit offset, 76 = 0x4c
    0, 0, 0, 0,                 // reserved
    // 1st retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x01, RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00, RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 2nd retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x02, RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00, RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 3rd retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x03, RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00, RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 4th retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x04, RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00, RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 5th retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x05, RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00, RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 6th retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x06, RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00, RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 7th retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x07, RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00, RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // exit retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00, RR_SEQ_ITEM_HDR(0, 0, 1, 0, 0, 0), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, FINISH),
};

/* read retry sequence table for Hynix 20nm nand chips */
U8 rr_seq_hynix_20nm[] = {
    0x01, 0x00, 0x82, 0x82,     // TAG 0x82820001
    8,                          // size of reg addr array
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, // reg addr array
    0x36,                       // update reg cmd
    0xac, 0,                    // exit offset, 172 = 0xac
    8, 0, 0, 0,                 // total rr level, only for hynix used
    // 1st retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x16,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 2nd retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x16,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 3rd retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x16,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 4th retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x16,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 5th retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x16,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 6th retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x16,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 7th retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x16,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 8th retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x16,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // exit retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, FINISH),
};

/* read retry sequence table for Hynix 26nm nand chips */
const U8 rr_seq_hynix_26nm[] = {
    0x01, 0x00, 0x82, 0x82,     // TAG 0x82820001
    4,                          // size of reg addr array
    0xa7, 0xad, 0xae, 0xaf,     // reg addr array
    0x36,                       // update reg cmd, used quite often, so put here
    0x52, 0,                    // exit offset, 82 = 0x52
    6, 0, 0, 0,                 // total RR level, only hynix used
    // 1st retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x06,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x0a, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x06,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x16,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 2nd retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x03,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x07, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x08,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x16,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 3rd retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x06,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x0d, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x0f,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x16,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 4th retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x09,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x14, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x17,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x16,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 5th retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x1a, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x1e,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x16,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 6th retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x00,
    RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x20, RR_SEQ_ITEM_HDR(0, 1, 1, 0, 1, 0), 0x25,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x16,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // exit retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, FINISH),
};

/* read retry sequence table for Toshiba A19nm nand chips */
const U8 rr_seq_toshiba_a19nm[] = {
    0x01, 0x00, 0x82, 0x82,     // TAG 0x82820001
    5,                          // size of reg addr array
    0x04, 0x05, 0x06, 0x07, 0x0d,   // reg addr array
    0x55,                       // update reg cmd, used quite often, so put here
    0x80, 0,                    // exit offset, 128 = 0x80
    0, 0, 0, 0,                 // reserved
    // pre condition
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x5c, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xc5,
    // 1st retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x04, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x04,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x7c, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x7e,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x26, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x5d,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 2nd retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x7c,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x78, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x78,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x26, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x5d,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 3rd retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x7c, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x76,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x74, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x72,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x26, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x5d,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 4th retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x08, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x08,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x26, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x5d,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 5th retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x08, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x7e,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x76, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x74,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x26, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x5d,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 6th retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x10, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x76,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x72, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x70,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x26, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x5d,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 7th retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x02, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x7e, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x7c, 
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb3,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x26, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x5d,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // exit retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xff,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, FINISH),
};

/* read retry sequence table for Toshiba 19nm nand chips */
const U8 rr_seq_toshiba_19nm[] = {
    0x01, 0x00, 0x82, 0x82,     // TAG 0x82820001
    4,                          // size of reg addr array
    0x04, 0x05, 0x06, 0x07,     // reg addr array
    0x55,                       // update reg cmd, used quite often, so put here
    0x66, 0,                    // exit offset, 102 = 0x66
    0, 0, 0, 0,                 // reserved
    // pre condition
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x5c, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xc5,
    // 1st retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x26, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x5d,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 2nd retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x04, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x04,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x04, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x04,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x26, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x5d,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 3rd retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x7c, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x7c,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x7c, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x7c,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x26, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x5d,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 4th retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x78, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x78,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x78, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x78,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x26, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x5d,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 5th retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x74, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x74,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x74, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x74,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x26, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x5d,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // 6th retry read
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x08, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x08,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x08, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x08,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x26, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x5d,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    // exit retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xff,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, FINISH),
};

/* read retry sequence table for SanDisk 19nm nand chips */
const U8 rr_seq_sandisk_19nm[] = {
    0x01, 0x00, 0x82, 0x82,     // TAG 0x82820001
    3,                          // size of reg addr array
    0x04, 0x05, 0x07,           // reg addr array
    0x53,                       // update reg cmd, used quite often, so put here
    0x5a, 0x1,                  // exit offset, 346 = 0x15A
    0, 0, 0, 0,                 // reserved
    // activation and initialization
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 0, RR_CMD), 0x04, 0x00, 
	RR_SEQ_ITEM_HDR(1, 1, 1, 1, 0, RR_CMD), 0x05, 0x00,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 0, RR_CMD), 0x06, 0x00, 
	RR_SEQ_ITEM_HDR(1, 1, 1, 1, 0, RR_CMD), 0x07, 0x00,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 0, RR_CMD), 0x08, 0x00, 
	RR_SEQ_ITEM_HDR(1, 1, 1, 1, 0, RR_CMD), 0x09, 0x00,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 0, RR_CMD), 0x0A, 0x00, 
	RR_SEQ_ITEM_HDR(1, 1, 1, 1, 0, RR_CMD), 0x0B, 0x00,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 0, RR_CMD), 0x0C, 0x00,
    // 1st retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xF0, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xF0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xF0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 2nd retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xEF, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 3rd retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xDF, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xD0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xD0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 4th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x1E, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x10,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 5th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x2E, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xD0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x20,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 6th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x3D, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xF0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x30,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 7th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xCD, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xD0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 8th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x0D, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xD0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x10,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 9th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x01, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x10,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x20,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 10th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x12, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x20,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x20,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 11th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xB2, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x10,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xD0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 12th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xA3, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x20,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xD0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 13th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x9F, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xD0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 14th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xBE, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xF0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xC0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 15th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xAD, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xC0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xC0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 16th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x9F, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xF0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xC0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 17th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x01, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 18th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x02, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 19th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x0D, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xB0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 20th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x0C, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xA0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // exit retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xff,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, FINISH),
};

/* read retry sequence table for SanDisk 1ynm nand chips */
const U8 rr_seq_sandisk_1ynm[] = {
    0x01, 0x00, 0x82, 0x82,     // TAG 0x82820001
    3,                          // size of reg addr array
    0x04, 0x05, 0x07,           // reg addr array
    0x53,                       // update reg cmd, used quite often, so put here
    0xFF, 0x01,                 // exit offset, 511 = 0x1FF
    0, 0, 0, 0,                 // reserved
    // activation and initialization
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 0, RR_CMD), 0x04, 0x00, 
	RR_SEQ_ITEM_HDR(1, 1, 1, 1, 0, RR_CMD), 0x05, 0x00,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 0, RR_CMD), 0x06, 0x00, 
	RR_SEQ_ITEM_HDR(1, 1, 1, 1, 0, RR_CMD), 0x07, 0x00,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 0, RR_CMD), 0x08, 0x00, 
	RR_SEQ_ITEM_HDR(1, 1, 1, 1, 0, RR_CMD), 0x09, 0x00,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 0, RR_CMD), 0x0A, 0x00, 
	RR_SEQ_ITEM_HDR(1, 1, 1, 1, 0, RR_CMD), 0x0B, 0x00,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 0, RR_CMD), 0x0C, 0x00,
    // 1st retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xF0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xF0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 2nd retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xF0, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x10,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 3rd retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE0, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xD0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 4th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x0F, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xF0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x20,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 5th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xFF, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 6th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xEF, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xD0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xF0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 7th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xDF, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xC0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 8th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x0E, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xF0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 9th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xFE, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 10th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xEE, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xD0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 11th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xDE, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xC0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 12th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xCE, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xB0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 13th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x11, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 14th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x01, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xF0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 15th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xF1, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x30,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 16th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE1, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xD0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x30,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 17th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x02, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xF0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x40,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 18th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x12, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x40,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 19th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xED, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xD0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 20th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xDD, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xC0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 21th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xCD, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xB0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 22th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xBD, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xA0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 23th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xEC, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xD0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 24th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xDC, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xC0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 25th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xBC, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xA0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 26th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xCC, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xB0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 27th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xCB, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xB0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 28th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xBB, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xA0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 29th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xAB, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x90,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xE0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 30th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xBA, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xA0,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xD0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // 31th retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xAA, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x90,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0xD0,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xB6, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE),
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, READ_PAGE_SUFFIX), 0XD6,
    // exit retry read
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0x3b, RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xb9,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00, RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 1, 1, 1, 1, RR_CMD), 0x00,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, RR_CMD), 0xff,
    RR_SEQ_ITEM_HDR(1, 0, 0, 0, 0, FINISH),
};

/**
  * run read retry sequence
  * @cs: nand chip select
  * @rr_cmd_type: store cmd in read retry sequence

  * return true if succeed, otherwise return false
  */
BOOL rr_seq_run(U32 cs, rr_cmd_type_e * rr_cmd_type)
{
    U8 cmd;
    U8 *addr;
    U8 *data;
    U8 pos = 0;

    while (1) {
        nand.rr_seq.status = RR_SEQ_ITEM_HDR_GET(nand.rr_seq.walker);

        /* run read retry sequence command */
        if (nand.rr_seq.status->cmd_exist) {
            switch (nand.rr_seq.status->cmd_type) {
            case RR_CMD:
                cmd = ((nand.rr_seq.status->update_cmd_in_tbl) ? (nand.rr_seq.update_cmd)
                       : (*(++(nand.rr_seq.walker))));
                NAND_printf("CMD:0x%02x ", cmd);
                if (!hal_nand_send_cmd(cs, cmd)) {
                    return FALSE;
                }
                break;
            case READ_PAGE:
                NAND_printf("read page level %d\n", nand.rr_seq.rr_cycle.level + 1);
                ++nand.rr_seq.walker;
                *rr_cmd_type = READ_PAGE;
                return TRUE;
            case FINISH:
                *rr_cmd_type = FINISH;
                return TRUE;
            default:
                printf("this type of sequence is not exsit, please check it!\n");
                return FALSE;
            }
        }

        /* run read retry sequence register address */
        if (nand.rr_seq.status->addr_exist) {
            if (pos == nand.rr_seq.addr_reg_num)
                pos = 0;
            addr = ((nand.rr_seq.status->addr_in_tbl)
                    ? (nand.rr_seq.addr_reg + pos)
                    : (U8 *) (++(nand.rr_seq.walker)));
            pos++;
            NAND_printf("ADDR:0x%02x ", *addr);
            if (!hal_nand_send_addr(cs, addr, 1)) {
                return FALSE;
            }
        }

        /* run read retry sequence write data */
        if (nand.rr_seq.status->data_exist) {
            data = (U8 *) (++(nand.rr_seq.walker));
            NAND_printf("DATA:0x%02x ", *data);
            hal_nand_wait_for_ready(cs);
            if (!hal_nand_write_raw_data(cs, data, 1, FALSE, 0, 0, 0)) {
                return FALSE;
            }
        }

        /* point to the next rr_seq_status */
        ++nand.rr_seq.walker;
    }
}

/**
  * rr_seq_run_page_read_suffix - run page read suffix command if needed, 
  * set retry level if retry sequence cycling is needed
  * @cs: nand chip select
  *
  * return true if succeed, otherwise return false
  */
BOOL rr_seq_run_page_read_suffix(U32 cs)
{
    U8 cmd;

    /* send read page suffix command if needed */
    if (RR_SEQ_CMD_TYPE_GET(nand.rr_seq.walker) == READ_PAGE_SUFFIX) {
        cmd = (*(++(nand.rr_seq.walker)));
        ++(nand.rr_seq.walker);

        NAND_printf("CMD:0x%02x ", cmd);
        if (!hal_nand_send_cmd(cs, cmd)) {
            return FALSE;
        }
    }

    /* point to the next rr level */
    nand.rr_seq.rr_cycle.prev = nand.rr_seq.rr_cycle.next;
    nand.rr_seq.rr_cycle.next = nand.rr_seq.walker;

    nand.rr_seq.rr_cycle.level++;
    nand.rr_seq.rr_cycle.cnt++;

    /* return to level0 when reached the max rr level when RR-level-cycling needed */
    if ((nand.rr_seq.rr_cycle.total_level > 0)  // Need RR-level-cycling
        && (nand.rr_seq.rr_cycle.level >= nand.rr_seq.rr_cycle.total_level)
        && (nand.rr_seq.rr_cycle.cnt <= nand.rr_seq.rr_cycle.total_level)) {    // Not all RR level cycled yet 
        nand.rr_seq.rr_cycle.level = 0;
        nand.rr_seq.walker = nand.rr_seq.body;  // To sub sequence of level0 
        nand.rr_seq.rr_cycle.next = nand.rr_seq.body;
    }

    /* 
     * Check if nand.rr_seq.rr_cycle.cnt exceeds retry total level, 
     * only used for RR-level-cycling 
     */
    if (nand.rr_seq.rr_cycle.total_level > 0 && nand.rr_seq.rr_cycle.cnt >= nand.rr_seq.rr_cycle.total_level) { // All RR level had be cycled. 
        rr_seq_goto_exit(cs);
    }

    return TRUE;
}

/**
  * go to read retry exit sequence
  **/
void rr_seq_goto_exit(U32 cs)
{
    /* point to exit sequence */
    nand.rr_seq.walker = nand.rr_seq.head + nand.rr_seq.exit_offset;
}

void rr_seq_reset_cycle_param(void)
{
    nand.rr_seq.rr_cycle.cnt = 0;

    /* 
     * Reset retry level when goto the exit sequence when retry levle cycling no needed, 
     * decrement to the present level when cycling needed 
     */
    if (nand.rr_seq.rr_cycle.total_level == 0) {    // RR-level-cycling not needed
        nand.rr_seq.rr_cycle.level = 0;
    } else {
        /* 
         * Back to the level which the previous RR dealed with, no matter it was a succuess of not. 
         */
        if (nand.rr_seq.rr_cycle.level == 0) {
            nand.rr_seq.rr_cycle.level = nand.rr_seq.rr_cycle.total_level - 1;
        } else if (nand.rr_seq.rr_cycle.level > 0) {
            nand.rr_seq.rr_cycle.level--;
        }
        nand.rr_seq.rr_cycle.next = nand.rr_seq.rr_cycle.prev;
    }
}

/**
  * set the entry point of retry sequence by retry sequence body pointer
  **/
void rr_seq_set_seq_entry(void)
{
    if (nand.rr_seq.rr_cycle.total_level > 0)   // RR-level-cycling not needed
        nand.rr_seq.walker = nand.rr_seq.rr_cycle.prev;
    else
        nand.rr_seq.walker = nand.rr_seq.body;
}

/**
  * hynix nand get read retry table from nand chip OTP
  * @ids: nand id for determin which nand chip we are using
  * @page_buf: buffer for reading page
  *
  * return TRUE when table found, otherwise return FALSE
  */
BOOL rr_get_rrt_from_nand(U8 * ids, U8 * page_buf)
{
    U8 *rrt_start_seq = (U8 *) 0;
    U8 *rrt_reg_addr = (U8 *) 0;
    U8 rrt_start_seq_UCG8T2A[] = { 0x36, 0xff, 0x40, 0xcc, 0x4d, 0x16, 0x17, 0x04, 0x19, 0x38 };
    U8 rrt_reg_addr_UCG8T2A[] = { 0xcc, 0xbf, 0xaa, 0xab, 0xcd, 0xad, 0xae, 0xaf };
    U8 rrt_start_seq_UCG8T2B[] = { 0x36, 0xae, 0x00, 0xb0, 0x4d, 0x16, 0x17, 0x04, 0x19, 0x38 };
    U8 rrt_reg_addr_UCG8T2B[] = { 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, };
    U8 rrt_start_seq_UBG8T2C[] = { 0x36, 0xae, 0x00, 0xb0, 0x4d, 0x16, 0x17, 0x04, 0x19, 0x38 };
    U8 rrt_reg_addr_UBG8T2C[] = { 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, };
    U8 otp_page_addr[5] = { 0x00, 0x00, 0x00, 0x02, 0x00 };
    U8 *rrt_load;

    int i, j, k;

    /* get RR sequence using nand id */
    if (ids[3] == 0xda) {
        rrt_start_seq = rrt_start_seq_UCG8T2A;
        rrt_reg_addr = rrt_reg_addr_UCG8T2A;
    } else if (ids[3] == 0xeb) {
        rrt_start_seq = rrt_start_seq_UCG8T2B;
        rrt_reg_addr = rrt_reg_addr_UCG8T2B;
    } else if (ids[3] == 0x91) {
        rrt_start_seq = rrt_start_seq_UBG8T2C;
        rrt_reg_addr = rrt_reg_addr_UBG8T2C;
    } else {
        rrt_start_seq = rrt_start_seq_UBG8T2C;
        rrt_reg_addr = rrt_reg_addr_UBG8T2C;
    }
    rrt_load = rr_seq_hynix_20nm + HYNIX_RR_SEQ_REG_ADDR_POS;
    for (i = 0; i < NAND_HYNIX_RRT_REG_NUM; i++)
        rrt_load[i] = rrt_reg_addr[i];

    /* send start cmd, addr and datas */
    nand_reset(0);
    hal_nand_send_cmd(0, rrt_start_seq[0]);
    hal_nand_send_addr(0, &rrt_start_seq[1], 1);
    hal_nand_write_raw_data(0, &rrt_start_seq[2], 1, FALSE, 0, 0, 0);
    hal_nand_send_addr(0, &rrt_start_seq[3], 1);
    hal_nand_write_raw_data(0, &rrt_start_seq[4], 1, FALSE, 0, 0, 0);
    hal_nand_send_cmd(0, rrt_start_seq[5]);
    hal_nand_send_cmd(0, rrt_start_seq[6]);
    hal_nand_send_cmd(0, rrt_start_seq[7]);
    hal_nand_send_cmd(0, rrt_start_seq[8]);

    /* read OTP page */
    hal_nand_send_cmd(0, NAND_READ_PAGE_CLE1);
    hal_nand_send_addr(0, otp_page_addr, 5);
    hal_nand_send_cmd(0, NAND_READ_PAGE_CLE2);
    hal_nand_wait_for_ready(0);
    hal_nand_read_raw_data(0, page_buf, NAND_HYNIX_OTP_LEN, FALSE);

    /* read OTP end */
    nand_reset(0);
    hal_nand_send_cmd(0, rrt_start_seq[9]);
    hal_nand_wait_for_ready(0);

    /* check if data is right */
    page_buf += 2;              // point to read retry data start
    for (i = 0; i < NAND_HYNIX_RRT_COPY_NUM; i++) {
        k = 1;
        for (j = 0; j < NAND_HYNIX_RRT_LEN; j++) {
            /* refill the rr_seq_hynix_20nm array using read retry table read from OTP */
            rrt_load = rr_seq_hynix_20nm + HYNIX_RR_SEQ_RRT_POS;    // point to head of hynix 20nm RR table
            rrt_load[k] = *(page_buf + j);  // set the data in hynix 20nm RR table
            if ((j + 1) % NAND_HYNIX_RRT_REG_NUM == 0)
                k += 5;
            else
                k += 2;
            if ((*(page_buf + j) ^ (*(page_buf + NAND_HYNIX_RRT_LEN + j))) != 0xff)
                break;
        }
        if (j == NAND_HYNIX_RRT_LEN) {
            printf("read retry table found success!\n");
            return TRUE;
        } else
            page_buf += NAND_HYNIX_RRT_LEN * 2; // point to the next copy of RRT and the iverse RRT
    }

    printf("read retry table not found!\n");
    return FALSE;
}

/**
  * rr_seq_get_len - get read retry sequence table length
  * @ids: nand ids to distingush which nand read retry sequence we will use
  *
  * return read retry sequence length if succeed, otherwise return 0
  */
U32 rr_seq_get_len(U8 ids[])
{
    switch (ids[0]) {
    case 0x2c:
        return ARRAY_SIZE(rr_seq_micron_20nm);
    case 0x45:
        if ((ids[5] & 0x07) == 0x07)
            return ARRAY_SIZE(rr_seq_sandisk_19nm);
        else if ((ids[5] & 0x07) == 0x00)
            return ARRAY_SIZE(rr_seq_sandisk_1ynm);
    case 0x98:
        if ((ids[5] & 0x07) == 0x07)
            return ARRAY_SIZE(rr_seq_toshiba_19nm);
        else if ((ids[5] & 0x07) == 0x00)
            return ARRAY_SIZE(rr_seq_toshiba_a19nm);
    case 0xad:
        if ((ids[5] & 0x07) == 0x04)
            return ARRAY_SIZE(rr_seq_hynix_20nm);
        else if ((ids[5] & 0x07) == 0x03)
            return ARRAY_SIZE(rr_seq_hynix_26nm);
    default:
        printf("can not find retry sequence because we donot support this vendor!\n");
        return 0;
    }
}

