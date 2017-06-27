#ifndef HAL_BCH_H
#define HAL_BCH_H

/*==================================================================================================
     Header Name: hal_bch.h
     General Description: Provide some defines for bch drivers.
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
                                        INCLUDE FILES
==================================================================================*/
#include <io.h>

/*================================================================================
                          Hash Defines And Macros
=================================================================================*/
#define ECC_STATUS_CHECK_TIMEOUT_MS		(10)    // set ecc status check timeout to be 10ms

/*==================================================================================================
                                             ENUMS
==================================================================================================*/
typedef enum _nand_ecc_levels {
    BCH_Ecc_0bit = 0,
    BCH_Ecc_2bit = 2,
    BCH_Ecc_4bit = 4,
    BCH_Ecc_6bit = 6,
    BCH_Ecc_8bit = 8,
    BCH_Ecc_10bit = 10,
    BCH_Ecc_12bit = 12,
    BCH_Ecc_14bit = 14,
    BCH_Ecc_16bit = 16,
    BCH_Ecc_18bit = 18,
    BCH_Ecc_20bit = 20,
    BCH_Ecc_22bit = 22,
    BCH_Ecc_24bit = 24,
    BCH_Ecc_26bit = 26,
    BCH_Ecc_28bit = 28,
    BCH_Ecc_30bit = 30,
    BCH_Ecc_32bit = 32,
    BCH_Ecc_34bit = 34,
    BCH_Ecc_36bit = 36,
    BCH_Ecc_38bit = 38,
    BCH_Ecc_40bit = 40,
    BCH_Ecc_42bit = 42,
    BCH_Ecc_44bit = 44,
    BCH_Ecc_46bit = 46,
    BCH_Ecc_48bit = 48,
    BCH_Ecc_50bit = 50,
    BCH_Ecc_52bit = 52,
    BCH_Ecc_54bit = 54,
    BCH_Ecc_56bit = 56,
    BCH_Ecc_58bit = 58,
    BCH_Ecc_60bit = 60,
    BCH_Ecc_62bit = 62,
    BCH_ECC_LEVEL_MAX = 62
} nand_ecc_levels_t;

/*=================================================================================
                                 STRUCTURES AND OTHER TYPEDEFS
=================================================================================*/
typedef struct {
    U32 total_page_sz;
    U32 block0_ecc_level;        // Ecc level for Block 0 - BCH
    U32 blockn_ecc_level;        // Ecc level for block n
    U32 block0_sz;          	 // Number of bytes for Block0 - BCH */
    U32 blockn_sz;               // Block size in bytes for all blocks other than Block0 - BCH
    U32 meta_sz;          		 // Metadata size - BCH
    U32 bch_type;                // GF13 0, GF14 1
    U32 blockn_num_per_page;     // number of blockn per page
} bchcfg_t;

/*=================================================================================
                                 Global FUNCTION PROTOTYPES
=================================================================================*/
void bch_update_ecc_param(void);
void bch_clear_complete_flag(void);
BOOL bch_set_flash_layout(U32 cs, bchcfg_t *bchcfg);
BOOL bch_check_ecc_status(U32 cs);
void bch_reset(void);
void bch_enable(void);

#endif /* HAL_BCH_H */
