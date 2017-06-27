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
                                        GLOBAL VARIABLES
==================================================================================*/
/* nor cmd set */
extern nor_cmdset_t nor_cmdset0;
extern nor_cmdset_t nor_cmdset1;

/* nor configure information table */
nor_cfg_t nor_cfg_table[] = {
	{ 
		.name = "M29W256G", 
		.id = { NOR_MFR_NUMONYX, 0x7e, 0x22, 0x01, }, 
		.block_size = BLOCK_SIZE_128KB, 
		.chip_size = CHIP_SIZE_32MB,
		.cmdset = &nor_cmdset0,
	},
	{ 
		.name = "MT28GU01GAAA2EGC", 		// ADmux
		.id = { NOR_MFR_MICRON, 0xB1, 0, 0 }, 
		.block_size = BLOCK_SIZE_256KB, 
		.chip_size = CHIP_SIZE_128MB,
		.cmdset = &nor_cmdset1,
	},
	{ 
		.name = "MT28GU01GAAA1EGC", 		// AD-nonmux
		.id = { NOR_MFR_MICRON, 0xB0, 0, 0 }, 
		.block_size = BLOCK_SIZE_256KB, 
		.chip_size = CHIP_SIZE_128MB,
		.cmdset = &nor_cmdset1,
	},
	{ 
		.name = "MT28EW01GABA", 
		.id = { NOR_MFR_MICRON, 0x7E, 0x28, 0x01 }, 
		.block_size = BLOCK_SIZE_128KB, 
		.chip_size = CHIP_SIZE_128MB,
		.cmdset = &nor_cmdset0,
	},
};

/* nor information, global variable */
nor_cfg_t *g_nor_cfg;

/* cpu boot configuration related to NOR flash */
static nor_bootcfg_t nor_boot[] = {
	{
		.cpuid = CHIP_MX6DQ_TO1_0,
		.firmware_load_addr = 0x920000,       // arik2, for ocram, (0x920000), for ddr, 0x27800000,
		.weim_init = (BOOL (*)(void))(0x000069c1),
		.hapi_src_reg = (U32 *)0x901ddc,
		.ivt_offset = 0x1000,
	},
	{
		.cpuid = CHIP_MX6SX_TO1_0,
		.firmware_load_addr = 0x97800000,         // for ocram, (0x8fc400)
		.weim_init = (BOOL (*)(void))(0x00005b99),
		.hapi_src_reg = (U32 *)0x900a28,
		.ivt_offset = 0x1000,
	},
	{
		.cpuid = CHIP_MX6SX_TO1_1,
		.firmware_load_addr = 0x97800000,     // for ocram, (0x8fc400)
		.weim_init = (BOOL (*)(void))(0x00005b99),
		.hapi_src_reg = (U32 *)0x900a28,
		.ivt_offset = 0x1000,
	},
	{
		.cpuid = CHIP_MX7D_TO1_0,
		.firmware_load_addr = 0x932000,     // for ocram, (0x932000) 
		.weim_init = (BOOL (*)(void))(0x000054a1),
		.hapi_src_reg = (U32 *)0x009074b0,
		.ivt_offset = 0x400,
	},
	{
		.cpuid = CHIP_MX6UL_TO1_0,
		.firmware_load_addr = 0x97800000,		  // for ocram, (0x100000), for ddr, 0x97800000
		.weim_init = (BOOL (*)(void))(0x00005835),
		.hapi_src_reg = (U32 *)0x900a2c,
		.ivt_offset = 0x1000,
	},
	{
		.cpuid = CHIP_MX6ULL_TO1_0,
		.firmware_load_addr = 0x97800000,		  // for ocram, (0x100000), for ddr, 0x97800000
		.weim_init = (BOOL (*)(void))(0x00005865),
		.hapi_src_reg = (U32 *)0x900a2c,
		.ivt_offset = 0x1000,
	},
	{
		.cpuid = CHIP_MX6SLL_TO1_0,
		.firmware_load_addr = 0x97800000,		  // for ocram, (0x100000), for ddr, 0x97800000
		.weim_init = (BOOL (*)(void))(0x00005865),
		.hapi_src_reg = (U32 *)0x900a2c,
		.ivt_offset = 0x1000,
	},
};

nor_bootcfg_t *g_nor_boot;
/*===================================================================================
                                        GLOBAL FUNCTIONS
==================================================================================*/
/*
 * nor_boot_chip_cfg - config and init weim interface by different cpu types
 *
 */
BOOL nor_boot_chip_cfg(chip_e chip) 
{
	int i, j;
	U32 weim_boot_cfg[] = {
		WEIM_CFG_AD16_MUXED,			// A-D muxed
		WEIM_CFG_ADL16_NONMUXED,   		// not muxed, 16L
		WEIM_CFG_ADH16_NONMUXED,     	// not muxed, 16H
		WEIM_CFG_AD32_MUXED,			// reserved
		WEIM_CFG_AD16_MUXED_7D,			// A-D muxed for 7D
		WEIM_CFG_ADL16_NONMUXED_7D,		// not muxed, 16L for 7D
	};

	for (i = 0; i < ARRAY_SIZE(nor_boot); i++) {
		if (nor_boot[i].cpuid == chip) {
			g_nor_boot = &nor_boot[i];

			/* set NOR by different iomux configuration */
			for(j = 0; j < ARRAY_SIZE(weim_boot_cfg); j++) {

				/* weim boot fuse configuration */
				*(g_nor_boot->hapi_src_reg) = weim_boot_cfg[j];
		
				/* call init function by hacking ROM code */
				if (!((*(g_nor_boot->weim_init))())) {
					printf("weim init failed!\n");
					return FALSE;
				}

				/* select nor flash cmdset first, cmdset will be different between different chips */
				if(nor_probe()) {
					return TRUE;
				}
			}

			if (j == ARRAY_SIZE(weim_boot_cfg)) {
				printf("weim configuration failed, nor flash can't be supported!\n");
				return FALSE;
			}
		}
	}

	printf("this cpu chip is not supported in the nor_boot_cfg list\n");
	return FALSE;
}

/*
 * nor_probe - select cmd set of nor flash
 * 
  * return true if succeed, otherwise return false
 */
BOOL nor_probe(void)
{
	int i;
	int idx;
	static U8 ids[4];

	for(i = 0; i < ARRAY_SIZE(nor_cfg_table); i++) {
		/* reset ids first */
		memset(ids, 0, sizeof(ids));

		/* read id first */
		(*(nor_cfg_table[i].cmdset->read_id))(ids);

		/* check device id */
		for(idx = 0; idx < ARRAY_SIZE(nor_cfg_table[i].id); idx++) {
			if(ids[idx] != nor_cfg_table[i].id[idx])
				break;
		}
		if(idx == ARRAY_SIZE(nor_cfg_table[i].id)) {
			printf("nor flash: %s found!\n", nor_cfg_table[i].name);
			g_nor_cfg = &nor_cfg_table[i];

			return TRUE;
		}
	}

	return FALSE;
}

/**
  * nor_cfg_dump - dump nor configuration information, 
  				   such as block_size, chip_size
  *
  * return true if succeed, otherwise return false
  */
void nor_cfg_dump(void)
{
	int i;

	printf("*************************************\n");
	printf("nor flash info:\n");

	/* dump ids */
	printf("ids: ");
	for(i = 0; i < ARRAY_SIZE(g_nor_cfg->id); i++)
		printf("0x%2x ", (g_nor_cfg->id)[i]);
	printf("\n");

	/* dump block size and chip size */
	printf("block size:\t%dkB\n", g_nor_cfg->block_size / 1024);
	printf("chip size:\t%dMB\n", g_nor_cfg->chip_size / (1024 * 1024));
}

