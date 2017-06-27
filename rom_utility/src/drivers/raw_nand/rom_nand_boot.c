/*=================================================================================

    Module Name:  pu_irom_nand_flash.c

    General Description: Limited IROM NAND Driver.

===================================================================================
                               Freescale Confidential Proprietary
                        Template ID and version: IL93-TMP-01-0106  Version 1.20
                        (c) Copyright Freescale, All Rights Reserved

                                Copyright: 2007-2011 FREESCALE, INC.
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
Shirley Ye                  11/May/2011     ENGR142968   Make BBM swap disabled at default
Shirley Ye                  17/Aug/2010     ENGR125745   added parameter for bbm byte position in metadata
Boris Shulman               15/Jul/2010     ENGR00125192 Arik Environment Setup
Fareed Mohammed             24/Mar/2010     ENGR00121599 Fixed toggle mode issues, ecc types mapping and merged 
                                                            dev_b06314_er3 branch for engr121764 fixes
Fareed Mohammed             17/Mar/2010     ENGR00121600 Fixed ROM fuse allocation to match with fixes done to sbmr mapping 
                                                            and ocotp allocation in rtl release 7.05.
Fareed Mohammed             03/09/2010      ENGR00118666 Added code to update toggle mode DDR nand timings from FCB
Geo McCullough (r9aadu)     08-12-2009      ENGR00118662 Removed ritaproto because N/A for codex
Yaniv Adiri (rm96583)       03-12-2009      ENGR00118867 Updated driver
Yaniv Adiri   (rm96583)     06-11-2009      ENGR00116995 Updated code for redundant NAND compatability
Boris Shulman (rm96330)     10-08-2009      ENGR00111570 Changes for Rita clock requirements
Boris Shulman (rm96330)     11-01-2009      ENGR00111569 Updated Rita SBMR definitions
Boris Shulman (rm96330)     06-01-2009      ENGR00111567 Added iomux configuration for Rita 
Jameer B Mulani (B11771)	03/12/2007	    ENGcm02114   Changes done for reading rest of boot image from 
											   		     block1

Jameer B Mulani (B11771)	07/11/2007	    ENGcm01173   Changes done for NFCv2

Jameer B Mulani(B11771)     19/09/2007      ENGcm00142   Added nand_data_read_next_boot_block 
		  												 function 

Surender Kumar(r66464)      24/08/1007      TLSbo95816   EDC Update
Surender Kumar (r66464)     21/05/2007      TLSbo92398 	 Initial implementation.

Portability: Portable to other compilers or platforms.

===================================================================================
                                        INCLUDE FILES
==================================================================================*/
#include  "rom_nand_boot.h"
#include  "hamming.h"
#include  "nand_read_retry.h"
#include  "nand_base.h"
#include  "hal_bch.h"
#include  "rom_nand_burn.h"
#include  "platform.h"

/*=========================================================================
                                      GLOBAL Variables
==========================================================================*/
/* read retry sequence included in fcb struct */
extern FCBStruct_t fcb;         // terry?? maybe we can use the original rr_seq to make nand burn size smaller

/* nand boot configuration table */
nand_bootcfg_t nand_boot_cfg[] = {
	{
		.cpuid = CHIP_MX6DQ_TO1_0,
		.ddr_addr = 0x920000,		// arik2, for ocram, (0x920000), for ddr, 0x27800000
		.option = SET(RAND_DISABLE, FCB_CHECK_HAMMING, BURN_NO_READ_RETRY),
		.init_func = (BOOL (*)(void))(0x00002ae1),	// nand init function, nand_boot_init
	},
	{
		.cpuid = CHIP_MX6SDL_TO1_2,
		.ddr_addr = 0x920000,		// rigel1.1, for ocram, (0x920000), for ddr, 0x27800000
		.option = SET(RAND_DISABLE, FCB_CHECK_HAMMING, BURN_NO_READ_RETRY),
		.init_func = (BOOL (*)(void))(0x00002ac9),	// nand init function, nand_boot_init
	},
	{
		.cpuid = CHIP_MX6SX_TO1_0,
		.ddr_addr = 0x97800000,         // for ocram, (0x8fc400)
		.option = SET(RAND_BY_FUSE, FCB_CHECK_HAMMING, BURN_NO_READ_RETRY),
		.init_func = (BOOL (*)(void))(0x00007719),	// nand init function, nand_boot_init
	},
	{
		.cpuid = CHIP_MX6SX_TO1_1,
		.ddr_addr = 0x97800000,		// for ocram, (0x8fc400)
		.option = SET(RAND_ENABLE, FCB_CHECK_ECC62, BURN_WITH_READ_RETRY),
		.init_func = (BOOL (*)(void))(0x00007719),	// nand init function, nand_boot_init
	},
	{
		.cpuid = CHIP_MX7D_TO1_0,
		.ddr_addr = 0x932000,		// for ocram, (0x932000)
		.option = SET(RAND_ENABLE, FCB_CHECK_ECC62, BURN_WITH_READ_RETRY),
		.init_func = (BOOL (*)(void))(0x00004a81),	// nand init function, nand_boot_init_param
	},
	{
		.cpuid = CHIP_MX6UL_TO1_0,
		.ddr_addr = 0x97800000,		// for ocram, (0x100000)
		//.ddr_addr = 0x100000,		// for ocram, (0x100000)
		.option = SET(RAND_DISABLE, FCB_CHECK_ECC40, BURN_WITH_READ_RETRY),
		.init_func = (BOOL (*)(void))(0x00007261),	// nand init function, nand_boot_init
	},
	{
		.cpuid = CHIP_MX6ULL_TO1_0,
		.ddr_addr = 0x80000000,		// for ocram, (0x100000)
		//.ddr_addr = 0x100000,		// for ocram, (0x100000)
		.option = SET(RAND_DISABLE, FCB_CHECK_ECC40, BURN_WITH_READ_RETRY),
        .init_func = (BOOL (*)(void))(0x000072e5),	// nand init function, nand_boot_init
	},
};

/*==========================================================================
                                     FUNCTIONS
==========================================================================*/
/**
  * find retry sequence by nand ids, and initial all members of nand.rr_seq struct
  * @len: nand read retry sequence length
  * @seq_src: pointer of source of retry sequence to be copied into fcb.read_retry_seq

  * return TRUE when sequence found, otherwise return FALSE
  **/
static BOOL nand_init_retry_seq(FCBStruct_t *pFCB)
{
    int i;

	/* copy read retry sequence from FCB to fcb.read_retry_seq to store it */
	memcpy(fcb.read_retry_seq, pFCB->read_retry_seq, sizeof(fcb.read_retry_seq));

    /* get seqence head pointer */
    nand.rr_seq.head = fcb.read_retry_seq;
    nand.rr_seq.walker = nand.rr_seq.head;

    /* check tag */
    nand.rr_seq.tag = *(nand.rr_seq.walker);
    nand.rr_seq.tag |= *(++(nand.rr_seq.walker)) << 8;
    nand.rr_seq.tag |= *(++(nand.rr_seq.walker)) << 16;
    nand.rr_seq.tag |= *(++(nand.rr_seq.walker)) << 24;
    if (RR_TAG != nand.rr_seq.tag) {
        printf("read retry sequnece tag error!\n");
        return FALSE;
    }

    /* get reg addr array and update reg cmd */
    nand.rr_seq.addr_reg_num = *(++(nand.rr_seq.walker));
    if (nand.rr_seq.addr_reg_num > RR_ADDR_REG_NUM_MAX) {
        printf("read retry address reg number exceed!\n");
        return FALSE;
    }
    for (i = 0; i < nand.rr_seq.addr_reg_num; i++)
        (nand.rr_seq.addr_reg)[i] = *(++(nand.rr_seq.walker));

    nand.rr_seq.update_cmd = *(++(nand.rr_seq.walker));

    /* get exit offset and skip reserve area */
    nand.rr_seq.exit_offset = *(++(nand.rr_seq.walker));
    nand.rr_seq.exit_offset += (*(++(nand.rr_seq.walker)) << 8);

    /* set struct retry level */
    nand.rr_seq.rr_cycle.level = 0;
	nand.rr_seq.rr_cycle.cnt = 0;
    nand.rr_seq.rr_cycle.total_level = *(++(nand.rr_seq.walker));

    nand.rr_seq.walker += 4;    // skip reserve area

    nand.rr_seq.body = nand.rr_seq.walker;  // point to sequence body
    nand.rr_seq.rr_cycle.prev = nand.rr_seq.body;
    nand.rr_seq.rr_cycle.next = nand.rr_seq.body;

    return TRUE;
}

BOOL nand_boot_chip_cfg(chip_e chip)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(nand_boot_cfg); i++) {
		if (chip == nand_boot_cfg[i].cpuid) {

			nand.bootcfg = &nand_boot_cfg[i];

			/* call init function to config iomux and timing */
			(*(nand.bootcfg->init_func))();

			return TRUE;
		}
	}

	printf("this cpu chip is not supported in the nand_boot_cfg list\n");
	return FALSE;
}

/*
 * nand_boot_fcb_search - search for FCB in nand device.
   Function will search for FCB in nand device starting from block 0
   with a search stride of 'search_stride'. Upon finding a valid FCB the function
   will copy the relevant fields to the nand_data structure.
   If no FCB is found after FCB_SEARCH_COUNT attempts function will return FAIL.
 * @search_stride: Number of pages to jump between searches
 * @nand_buffer: nand buffer address
 * @pFCB: pointer to FCB address
 *
 * return TRUE if succeed, otherwise return FALSE
 */
BOOL nand_boot_fcb_search(U32 search_stride, U8 * nand_buffer, FCBStruct_t ** pFCB)
{
    U32 search_page = 0;
    U32 cnt;

    /* search for boot block up to 'search_count' times */
    for (cnt = 0; cnt < (nand.info.boot_search_limit); ++cnt) {
        /* read fcb page, always just one page */
        if (nand_read_multi_page(search_page, nand_buffer, 1)) {
			if (GET(FCB_CHECK, nand.bootcfg->option) == FCB_CHECK_HAMMING)
            	*pFCB = (FCBStruct_t *) (nand_buffer + NAND_HC_ECC_OFFSET_DATA_COPY);
			else
	            *pFCB = (FCBStruct_t *) nand_buffer;

            /* check fcb finger print */
            if (((*pFCB)->FingerPrint == FCB_FINGERPRINT) && ((*pFCB)->Version == FCB_VERSION)) {
                return TRUE;
            }
        }
        /* increment search page number for the next iteration */
        search_page += (nand.info.pages_per_block);
    }

    return FALSE;
}

/**
  * nand_boot_init_param - initialize nand_data struct and other parameter used when searching fcb
  *
  * return true if succeed, otherwise return false
  */
BOOL nand_boot_init_param(void)
{
#if 1
    nand.page_buffer = (U8 *)0x80002000;
#endif

	/* initialize boot search limit, we can get search limit by input argument, no need to depend on boot pins */
    nand.info.boot_search_limit = FCB_COPIES_NUM;

    /* initialize bbm realated */
    nand.info.disbbm = 1;

    /* initialize nand flashlayout related */
	if (GET(FCB_CHECK, nand.bootcfg->option) == FCB_CHECK_HAMMING) {
		nand.info.hamming_en = TRUE;
   		nand.info.ecc_en = FALSE;
    	nand.info.data_page_sz = 2048;
    	nand.info.total_page_sz = 2048 + 64; // 2112B, hamming default setting
	} else if (GET(FCB_CHECK, nand.bootcfg->option) == FCB_CHECK_ECC62) {
		nand.info.hamming_en = FALSE;
    	nand.info.ecc_en = TRUE;
    	nand.info.total_page_sz = 1024 + 1024;
    	nand.info.data_page_sz = 1024;

    	nand.info.bchcfg.meta_sz = 32;
    	nand.info.bchcfg.block0_sz = 128;
    	nand.info.bchcfg.blockn_sz = 128;
    	nand.info.bchcfg.blockn_num_per_page = 7;
    	nand.info.bchcfg.block0_ecc_level = BCH_Ecc_62bit;
    	nand.info.bchcfg.blockn_ecc_level = BCH_Ecc_62bit;
    	nand.info.bchcfg.bch_type = BCH_GF13_SUPPORTED;
    	nand.info.bchcfg.total_page_sz = 1024 + 1024;
	} else if (GET(FCB_CHECK, nand.bootcfg->option) == FCB_CHECK_ECC40) {
		nand.info.hamming_en = FALSE;
    	nand.info.ecc_en = TRUE;
    	nand.info.total_page_sz = 1024 + 1024;
    	nand.info.data_page_sz = 1024;

    	nand.info.bchcfg.meta_sz = 32;
    	nand.info.bchcfg.block0_sz = 128;
    	nand.info.bchcfg.blockn_sz = 128;
    	nand.info.bchcfg.blockn_num_per_page = 7;
    	nand.info.bchcfg.block0_ecc_level = BCH_Ecc_40bit;
    	nand.info.bchcfg.blockn_ecc_level = BCH_Ecc_40bit;
    	nand.info.bchcfg.bch_type = BCH_GF13_SUPPORTED;
    	nand.info.bchcfg.total_page_sz = 1024 + 1024;
	}

    /* initialize read retry related */
    nand.info.read_retry_en = FALSE;

    /* nand randomizer related */
	if (GET(RAND, nand.bootcfg->option) == RAND_BY_FUSE) {
    	nand.info.rand_en = ROM_OCOTP_RANDOMIZER_ENABLE_VALUE();
    	nand.info.rand_type = ROM_OCOTP_RANDOMIZER_TYPE_VALUE();
	} else if (GET(RAND, nand.bootcfg->option) == RAND_ENABLE) {
    	nand.info.rand_en = 1;
    	nand.info.rand_type = 2;
	} else if (GET(RAND, nand.bootcfg->option) == RAND_DISABLE) {
    	nand.info.rand_en = 0;
    	nand.info.rand_type = 2;
	}

    return TRUE;
}

/**
  * nand_boot_set_param : set nand_data and other parameters using fcb
  * @pFCB: point to fcb struct
  *
  * return true if succeed, otherwise return false 
  */
BOOL nand_boot_set_param(FCBStruct_t * pFCB)
{
    NAND_Timing_t zNANDTiming;

    /* nand flash layout related */
	nand.info.hamming_en = FALSE;
    nand.info.ecc_en = TRUE;
    nand.info.data_page_sz = pFCB->block0_sz + (pFCB->blockn_sz) * (pFCB->blockn_num_per_page);
    nand.info.total_page_sz = pFCB->total_page_sz;
    nand.info.pages_per_block = pFCB->pages_per_block;

	/* nand bch layout related */
    nand.info.bchcfg.meta_sz = pFCB->meta_sz;
    nand.info.bchcfg.block0_sz = pFCB->block0_sz;
    nand.info.bchcfg.blockn_sz = pFCB->blockn_sz;
    nand.info.bchcfg.block0_ecc_level = (pFCB->block0_ecc_level) << 1;
    nand.info.bchcfg.blockn_ecc_level = (pFCB->blockn_ecc_level) << 1;
    nand.info.bchcfg.blockn_num_per_page = pFCB->blockn_num_per_page;
    nand.info.bchcfg.bch_type = pFCB->bch_type;
    nand.info.bchcfg.total_page_sz = pFCB->total_page_sz;

    /* nand bad block marker related */
    nand.info.disbbm = pFCB->disbbm;
    nand.info.bbm_bit_off = pFCB->BadBlockMarkerStartBit;
    nand.info.bbm_byte_off_in_data = pFCB->BadBlockMarkerByte;
    nand.info.bbm_physical_off = pFCB->bbm_physical_off;
    nand.info.bbm_spare_offset = pFCB->bbm_spare_offset;

    /* nand randomizer related */
	if (GET(RAND, nand.bootcfg->option) == RAND_ENABLE)
	    nand.info.rand_en = pFCB->rand_en;

    /* read retry sequence get from FCB */
    nand.info.read_retry_en = (BOOL) (pFCB->read_retry_en);
    nand.info.read_retry_seq_len = pFCB->read_retry_seq_len;

    if (nand.info.read_retry_en && nand.info.read_retry_seq_len
		&& !nand_init_retry_seq(pFCB)) {
        return FALSE;
    }

    /* set nand timings, only use async mode */
    zNANDTiming = pFCB->m_NANDTiming;
    gpmi_set_nand_timing(&zNANDTiming, 0);

    return TRUE;
}

/**
  * nand_boot_init - nand boot, read fcb, then set nand_data
  *
  * return TRUE if succeed, otherwise return FALSE
  */
BOOL nand_boot_init(void)
{
    FCBStruct_t *pFCB;
    U32 search_stride;

    /* init nand_data struct for searching fcb */
    if (!nand_boot_init_param()) {
        return FALSE;
    }

    /* search fcb */
    search_stride = nand.info.pages_per_block;
    if (!nand_boot_fcb_search(search_stride, nand.page_buffer, (FCBStruct_t **) & pFCB)) {
        printf("fcb not found!\n");
        return FALSE;
    } else
        printf("fcb found success!\n");

    /* set nand_data struct by parameters reading from fcb */
    if (!nand_boot_set_param(pFCB))
        return FALSE;

    return TRUE;
}

