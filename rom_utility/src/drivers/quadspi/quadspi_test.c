/*
 * Copyright (C) 2011-2012, Freescale Semiconductor, Inc. All Rights Reserved
 * THIS SOURCE CODE IS CONFIDENTIAL AND PROPRIETARY AND MAY NOT
 * BE USED OR DISTRIBUTED WITHOUT THE WRITTEN PERMISSION OF
 * Freescale Semiconductor, Inc.
*/

/*!
 * @file quadspi_test.c
 * @brief Test for Quadspi interface.
 *
 */

#include "io.h"
#include "rom_symbols.h"
#include "qspi_private.h"
#include "qspi.h"

#define SPANSION_MAN_ID 0x20
#define N25Q128A_DEV_ID 0x19BA

extern void psp_qspi_iomux_config(uint32_t index);
extern int get_chip_type(void);
extern uint32_t qspi_get_instance(uint32_t index);
extern uint32_t qspi_map_memory_base(uint32_t instance);

extern chip_e chip;

static uint32_t qspi_init_flag_g = 0;

static void printQspiSpecs(unsigned int id)
{
    unsigned int manId = id & 0xFF;
    unsigned int devId = (id >> 8) & 0xFFFF;
    switch(manId){
	case SPANSION_MAN_ID:
	    printf("Micron Manufacturer\n");
            switch(devId){
		case N25Q128A_DEV_ID:
		    printf("N25Q128A - 256Mb\n");
		break;
	    }
	break;
    }
}

int quadspi_test_init(uint32_t index)
{

    unsigned int tmp;
    chip_e chip_type = CHIP_MAX;
    QSPI_MemMapPtr instance = NULL;
    f_v_u32_u32 dp_qspi_clk_cfg_func = NULL;
    f_v_u32 sp_qspi_clk_cfg_func = NULL;

    if (1 == qspi_init_flag_g)
        return 0;

    printf("QuadSPI Init....\n");

    if ((chip >= CHIP_MX7D_TO1_0) && (chip < CHIP_MX7D_MAX)) {
        sp_qspi_clk_cfg_func = (f_v_u32)psp_rom_get_symble(ROM_SYM_f_cfg_qspi_clock, chip);
        if (sp_qspi_clk_cfg_func)
            sp_qspi_clk_cfg_func(0);    //0: CLK_INIT, 20MHz
        else
            printf("Failed to get sp_qspi_clk_cfg_func address\n");
    } else {
        dp_qspi_clk_cfg_func = (f_v_u32_u32)psp_rom_get_symble(ROM_SYM_f_cfg_qspi_clock, chip);
        if (dp_qspi_clk_cfg_func)
            dp_qspi_clk_cfg_func(index, 0); //0: CLK_INIT, 18MHz
        else
            printf("Failed to get sp_qspi_clk_cfg_func address\n");
    }

    psp_qspi_iomux_config(index);

    chip_type = get_chip_type();
    if ((chip_type < CHIP_MX6SX_TO1_0) || (chip_type > CHIP_MX6ULL_MAX)) {
        printf("only pele, 6UL, 6ULL and ULT1 supports QSPI feature!!\n");
        return 1;
    }

    instance = (QSPI_MemMapPtr)qspi_get_instance(index);
    if (NULL == instance) {
        printf("invalid index: %d\n", index);
        return 1;
    }

    qspi_init(instance);
    tmp = (unsigned int)qspi_read_id(instance);
    printf("QuadSPI Reading ID = 0x%08x\n", tmp);
    printQspiSpecs(tmp);

    qspi_init_flag_g = 1;

    return 0;
}

//#define QSPI_WIRTE_DEBUG

int quadspi_test_program(uint32_t index, uint32_t src, uint32_t size, uint32_t seek, uint32_t skip)
{
    QSPI_MemMapPtr instance = NULL;
    uint32_t ret = 0;
    uint32_t i;
    uint32_t temp_dst = 0, temp_src = 0, tlen = 0;
#ifdef QSPI_WIRTE_DEBUG
    uint32_t prog_data = 0x87654320;
#endif

#ifdef QSPI_WIRTE_DEBUG
    for (i=0; i<(tlen/4); i++) {
        *(uint32_t *)(temp_src + i * 4) = prog_data++;
    }
#endif

    instance = (QSPI_MemMapPtr)qspi_get_instance(index);
    if (NULL == instance) {
        printf("invalid index: %d\n", index);
        return 1;
    }

    if (0 != quadspi_test_init(index)) {
        printf("qspi init failure\n");
        return 1;
    }

#ifdef QSPI_WIRTE_DEBUG
    temp_src = 0x8fc0000;
    tlen = 0x900000 - 0x8fc0000;

    ret = qspi_program(instance, 0, temp_src, tlen);
    if (ret) {
        printf("qspi program failure");
        return 1;
    }

    temp_dst = qspi_map_memory_base((uint32_t)instance);
    prog_data = 0x87654321;

    for (i=0; i<(tlen/4); i++) {
        if (*(uint32_t *)(temp_dst + (i * 4)) != prog_data++) {
            printf("qspi program date check error\n");
            return 1;
        }
    }

    printf("qspi program data check pass!!\n");

    return 0;
#else
    temp_src = src + skip;
    tlen = size - skip;

    ret = qspi_program(instance, seek, temp_src, tlen);
    if (ret) {
        printf("qspi program failure");
        return 1;
    }

    temp_dst = qspi_map_memory_base((uint32_t)instance);
    temp_dst += seek;

    for (i=0; i<(tlen/4); i++) {
        if (*(uint32_t *)(temp_dst + (i * 4)) != *(uint32_t *)(temp_src + (i * 4))) {
            printf("\n\nqspi program date check error\n");
            return 1;
        }
    }

    printf("\n\nqspi program data check pass!!\n");

    return 0;
#endif
}

int quadspi_test_erase_whole(uint32_t index)
{
    QSPI_MemMapPtr instance = NULL;

    instance = (QSPI_MemMapPtr)qspi_get_instance(index);
    if (NULL == instance) {
        printf("invalid index: %d\n", index);
        return 1;
    }

    if (0 != quadspi_test_init(index)) {
        printf("qspi init failure\n");
        return 1;
    }

    qspi_chip_erase(instance);

    return 0;
}

int quadspi_test_erase_sector(uint32_t index, uint32_t addr)
{
    QSPI_MemMapPtr instance = NULL;

    instance = (QSPI_MemMapPtr)qspi_get_instance(index);
    if (NULL == instance) {
        printf("invalid index: %d\n", index);
        return 1;
    }

    if (0 != quadspi_test_init(index)) {
        printf("qspi init failure\n");
        return 1;
    }

    qspi_sector_erase(instance, addr);

    return 0;
}

int quadspi_test_data_check(uint32_t index, uint32_t src, uint32_t size, uint32_t init, uint32_t seek, uint32_t skip)
{
    QSPI_MemMapPtr instance = NULL;
    uint32_t i;
    uint32_t temp_dst = 0, temp_src = 0;;

    instance = (QSPI_MemMapPtr)qspi_get_instance(index);
    if (NULL == instance) {
        printf("invalid index: %d\n", index);
        return 1;
    }

    if (init) {
        if (0 != quadspi_test_init(index)) {
            printf("qspi init failure\n");
            return 1;
        }
    }

    temp_dst = qspi_map_memory_base((uint32_t)instance);
    temp_dst += seek;

    temp_src = src + skip;

    for (i=0; i<(size/4); i++) {
        if (*(uint32_t *)(temp_dst + (i * 4)) != *(uint32_t *)(temp_src + (i * 4))) {
            printf("\n\nqspi program date check error, dst=0x%x, src=0x%x\n",
                    (temp_dst + (i * 4)), (temp_src + (i * 4)));
            return 1;
        }
    }

    printf("\n\nqspi program data check pass!!\n");

    return 0;
}


