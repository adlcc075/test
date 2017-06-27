#ifndef NAND_BASE_H
#define NAND_BASE_H

/*=================================================================================

    Module Name:  nand_base.h

    General Description: Header file Limited IROM NFC Driver.

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
Author (core ID)                Date          Number     Description of Changes
-------------------------   ------------    ----------   --------------------------
Fareed Mohammed             24-Mar-2010    ENGR00121599 Add new enum for BCH32 and BCH20.
Fareed Mohammed             09-Mar-2010    ENGR00118666 Added boot support for toggle mode DDR NAND
Geo McCullough (r9aadu)     08-12-2009      ENGR00118662 Removed ritaproto because N/A for codex
Yaniv Adiri (rm96583)       03-12-2009      ENGR00118867 Updated driver
Boris Shulman (rm96330)     05-10-2009     ENGR00115433 Add WEIM/NAND muxing
Yaniv Adiri (rm96583)       08-11-2009      ENGR00116995 Updated for Rita NFC
Boris Shulman (rm96330)     15-06-2009      ENGR00111569 Updated Rita SBMR definitions
Boris Shulman (rm96330)     06-01-2009      ENGR00111567 Added iomux configuration for Rita
Jameer B MUlani(B11771)     04-07-2008      ENGcm06425   changed 4bit and 8 bit ECC mode defines for ELVIS TO2. 
Jameer B Mulani(B11771)     22-05-2008      ENGcm05789  Added 10 ms of delay  define to support LBA nand.
Jameer B Mulani(B11771)     15-02-2008      ENGcm03394  RTL fix : 8 bit ECC,added definr for ECC_MODE_MASK
Jameer B Mulani(B11771)     3-01-2008       ENGcm02634  Setting ST_CMD default value to 0x70
Jameer B Mulani (B11771)    24/12/2007      ENGcm02512  RTL fixes  done for NFCv2 :
                                                        Clearing the Config2 reg val before cofiguring it.
Jameer B Mulani (B11771)    07/11/2007      ENGcm01173  Changes done for NFCv2
Surender Kumar (r66464)     24/08/2007      TLSbo95816  Initial implementation.

Portability: Portable to other compilers or platforms.

===================================================================================
                                        INCLUDE FILES
==================================================================================*/
#include "hal_dma.h"
#include "hal_gpmi.h"
#include "hal_bch.h"
#include "nand_read_retry.h"
/*=================================================================================
=
                                 LOCAL FUNCTION PROTOTYPES
=================================================================================*/

/*================================================================================
                          Hash Defines And Macros
=================================================================================*/
/************************************************************/
/*            NAND Flash vendors                            */
/************************************************************/
#define NAND_MFR_MICRON 	(0X2C)
#define NAND_MFR_SAMSUNG 	(0XEC)
#define NAND_MFR_HYNIX 		(0XAD)
#define NAND_MFR_TOSHIBA 	(0X98)
#define NAND_MFR_SANDISK 	(0X45)
#define NAND_MFR_SPANSION 	(0X01)

/************************************************************/
/*            NAND Flash Commands                           */
/************************************************************/
#define ONFI_SET_FEATURE_SIZE  2
#define ONFI_SET_FEATURE_CLE   0xEF
#define ONFI_SET_FEATURE_ALE   0x01
#define ONFI_FEATURE_SIZE  4

#define NAND_READ_PAGE_CLE1 	0x00
#define NAND_READ_PAGE_CLE2 	0x30

#define NAND_PROGRAM_PAGE_CLE1 	0x80
#define NAND_PROGRAM_PAGE_CLE2 	0x10
#define NAND_PROGRAM_STATUS_FAIL 0x01

#define NAND_ERASE_BLOCK_CLE1 	0x60
#define NAND_ERASE_BLOCK_CLE2 	0xd0
#define NAND_ERASE_STATUS_FAIL 	0x01
#define NAND_WP_ENABLE 	0x80

#define NAND_READ_STATUS_CLE	0x70
#define NAND_RESET_CLE 			0xff
#define NAND_READ_ID_CLE		0x90
#define NAND_SET_FEATURE_CLE	0xEF
#define NAND_GET_FEATURE_CLE	0xEE

#define NAND_UNLOCK_LOWER_BLOCK 	0x23
#define NAND_UNLOCK_UPPER_BLOCK 	0x24

/****************************************************/
/*     Hynix nand read retry table from OTP         */
/****************************************************/
#define NAND_HYNIX_OTP_LEN		(1026)
#define NAND_HYNIX_RRT_COPY_NUM	(8)
#define NAND_HYNIX_RRT_LEN		(64)
#define NAND_HYNIX_RRT_REG_NUM 	(8)
#define HYNIX_RR_SEQ_REG_ADDR_POS (5)
#define HYNIX_RR_SEQ_RRT_POS	(20)

/************************************************************/
/*            NAND flash parameters            */
/************************************************************/
/* GF13 and GF14 */
#define BCH_GF14_SUPPORTED 		(1)
#define BCH_GF13_SUPPORTED 		(0)

/* bad block marker */
#define BAD_BLOCK_MARKER_BYTE 	(0xFF)
#define BLOCK_MARK_BAD_BYTE		(0x00)

/************************************************************/
/*            NAND Configuration values from fuses          */
/************************************************************/
#define hapi_fuse_box_bank0		(0x021BC400)
#define HW_OCOTP_REG_ADDR(n)	((UINT8*)hapi_fuse_box_bank0 + ((n) * 0x10))
#define HW_OCOTP_REG_RD(n)		(*(volatile U32 *)HW_OCOTP_REG_ADDR(n))

/* NAND Randomizer enable fuse*/
#define ROM_OCOTP_RANDOMIZER_ENABLE_MASK  (0x00000080)
#define ROM_OCOTP_RANDOMIZER_ENABLE_SHIFT ((UINT8)7)
#define ROM_OCOTP_RANDOMIZER_ENABLE_VALUE() \
    ((HW_OCOTP_REG_RD(45)&ROM_OCOTP_RANDOMIZER_ENABLE_MASK) >> ROM_OCOTP_RANDOMIZER_ENABLE_SHIFT)

/* NAND Randomizer type fuse*/
#define ROM_OCOTP_RANDOMIZER_TYPE_MASK  (0x00006000)
#define ROM_OCOTP_RANDOMIZER_TYPE_SHIFT ((UINT8)13)
#define ROM_OCOTP_RANDOMIZER_TYPE_VALUE() \
    ((HW_OCOTP_REG_RD(45)&ROM_OCOTP_RANDOMIZER_TYPE_MASK) >> ROM_OCOTP_RANDOMIZER_TYPE_SHIFT)

/*==================================================================================================
                                 STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/
typedef struct {
    U32 boot_search_limit;
    U32 data_page_sz;           /*data pagesize */
    U32 total_page_sz;
    U32 pages_per_block;

	bchcfg_t bchcfg;

    U32 bbm_byte_off_in_data; //!< FCB value that gives byte offset for bad block marker in data area
    U32 bbm_bit_off;        //!< FCB value that gives starting bit offset within m_u32bbm_byte_off_in_data
    U32 bbm_physical_off;
    U32 bbm_spare_offset;    //BBM data position stored at metadata
    U32 disbbm;

    BOOL read_retry_en;
    U32 read_retry_seq_len;

    BOOL hamming_en;
    BOOL ecc_en;    
    BOOL rand_en;
    U32 rand_type;

} nand_info_t;

typedef struct {
	U32 cpuid;
	U32 ddr_addr;
	U32 option;
	BOOL (*init_func)(void);	// nand boot init function, mainly for timing and iomux

} nand_bootcfg_t;

typedef struct {
	/* nand flash info, such as page size */
	nand_info_t info;

	/* nand boot configuration, depends on differnent chips */
	nand_bootcfg_t *bootcfg;

	/* 16k iram buffer for nand page read and write use */
#ifdef MX6X
	//U8 page_buffer[8936];
    U8 *page_buffer;
#else
#ifdef MX7X
	U8 page_buffer[17664];
#endif
#endif

	/* nand aux buffer */   
	U8 aux_buffer[256];

	/* read retry sequnece parameter struct */
	rr_seq_param_t rr_seq;

} nand_t;

extern nand_t nand;

typedef enum {
	RAND_DISABLE = 1,
	RAND_ENABLE,
	RAND_BY_FUSE,
} rand_e;

typedef enum {
	FCB_CHECK_HAMMING = 1,
	FCB_CHECK_ECC62,
	FCB_CHECK_ECC40,
} fcb_check_e;

typedef enum {
	BURN_NO_READ_RETRY = 1,
	BURN_WITH_READ_RETRY,
} read_retry_e;

/*=================================================================================
                                 Global Functions
=================================================================================*/
BOOL nand_reset(U32 cs);
BOOL nand_interface_init(void);
BOOL nand_read_page_no_retry(U32 cs, U32 page_addr, U8 * page_buf);
BOOL nand_read_page(U32 cs, U32 page_addr, U8 * page_buf);
BOOL nand_read_multi_page(U32 page_addr, U8 * page_buf, U32 size);
BOOL nand_get_bbm(U32 cs, U32 page_addr, U8 * buf);
BOOL is_block_bad_by_bbm(U32 cs, U32 block_addr);
U8 nand_read_status(U32 cs);
BOOL nand_write_page(U32 cs, U32 page_num, U8 * page_buf);
BOOL nand_write_page_byte(U32 cs, U32 page_addr, U32 offset, U8 byte);
BOOL nand_erase_block(U32 cs, U32 block_addr);
BOOL nand_read_id(U32 cs, U8 addr, U8 * ids);
BOOL nand_set_feature(U32 cs, U8 addr, U8 * paras);
BOOL nand_get_feature(U32 cs, U8 addr, U8 * paras);
BOOL nand_mark_block_bad(U32 cs, U32 block_addr);

#endif /* NAND_BASE_H */
