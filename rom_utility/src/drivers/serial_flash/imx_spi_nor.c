/*
 * Copyright (C) 2010-2012, Freescale Semiconductor, Inc. All Rights Reserved
 * THIS SOURCE CODE IS CONFIDENTIAL AND PROPRIETARY AND MAY NOT
 * BE USED OR DISTRIBUTED WITHOUT THE WRITTEN PERMISSION OF
 * Freescale Semiconductor, Inc.
*/

#include "io.h"
#include "spi/imx_spi.h"
#include "imx_spi_nor.h"

#define AT45DB321D          1
#define M25P32               2

#define SPI_NOR_XFER_SZ     512

extern int spi_nor_write_atmel(u32 addr, u8 * data, u32 length);
extern int spi_nor_read_atmel(void *src, void *dest, int len);
extern int spi_nor_erase_atmel(u32 addr, u32 length);
extern int spi_nor_write_numonyx(u32 addr, u8 * data, u32 length);
extern int spi_nor_read_numonyx(void *src, void *dest, int len);
extern int spi_nor_erase_numonyx(u32 addr, u32 length);

// Buffers for low level drivers
u8 __attribute__ ((aligned(4))) g_spi_nor_tx_buf[SPI_NOR_XFER_SZ];
u8 __attribute__ ((aligned(4))) g_spi_nor_rx_buf[SPI_NOR_XFER_SZ];

// Functions for low level drivers
//imx_spi_init_func_t spi_init_flash;
imx_spi_xfer_func_t spi_xfer_flash;

static struct spi_v2_3_reg spi_nor_reg;
struct imx_spi_dev imx_spi_nor = {
  reg:&spi_nor_reg,
  fifo_sz: 32,
};

spinor_info_t nor_list[] = {
    {
     {0x01, 0x27, 0x1f},
     "AT45DB321D",
     ATMEL_AT45DB321D,     
     spi_nor_erase_atmel,
     spi_nor_read_atmel,
     spi_nor_write_atmel,
    },
    {
     {0x16, 0x20, 0x20},
     "M25P32",
     MICRON_M25P32,
     spi_nor_erase_numonyx,
     spi_nor_read_numonyx,
     spi_nor_write_numonyx,
    },
    {
     {0x16, 0xba, 0x20},
     "N25Q32",
     MICRON_N25Q32,
     spi_nor_erase_numonyx,
     spi_nor_read_numonyx,
     spi_nor_write_numonyx,
    },
    {
     {0x00, 0x23, 0x1f},
     "AT45DB021E",
     ATMEL_AT45DB021E,
     spi_nor_erase_atmel,
     spi_nor_read_atmel,
     spi_nor_write_atmel,
    },
};

spinor_info_p spinor = NULL;
//spinor_info_p spinor;

int spi_nor_get_id(u32 * data)
{
    /* query id */
    g_spi_nor_tx_buf[3] = JEDEC_ID; // byte3 is the 1st to shift out
    spi_xfer_flash(&imx_spi_nor, g_spi_nor_tx_buf, g_spi_nor_rx_buf, 4 * 8);

    *data = *((u32 *)g_spi_nor_rx_buf);

    return 0;
}

int spi_nor_init(u32 base, u32 ss)
{
    u8 id[4];//id是一个数组
    int i;
    int j;

    spinor = NULL;//define a 结构体 spinor

    spi_xfer_flash = imx_ecspi_xfer;//将spi_xfer_flash指向imx_ecspi_xfer()函数
    //spi_init_flash = imx_ecspi_init; 

    imx_spi_nor.base = base;//将base的值赋给imx_spi_dev结构体类型的变量的base元素
    imx_spi_nor.ss = ss;//将ss的值赋给imx_spi_dev结构体类型的变量的ss元素

    /*
      SPI driver saved the ctrl/cfg register to spi_nor_reg when initializing SPI, they
      were used in imx_spi_start which was called by every single transfer.

      Although we hacked ROM's init routine, the registers should be saved.
     */
    spi_nor_reg.ctrl_reg = readl(base + 0x08);
    spi_nor_reg.cfg_reg = readl(base + 0x0C);

    spi_nor_get_id((u32 *)id);
    
    for(j=0;j<3;j++) {
        printf("\nspinor id[%d] is: 0x%x\n", j, id[j]);
    }

    for (i = 0; i < ARRAY_SIZE(nor_list); i++) {
        if ((id[0] == nor_list[i].id.id0) && (id[1] == nor_list[i].id.id1)
            && (id[2] == nor_list[i].id.id2)) {
            spinor = &nor_list[i];
            break;
        }
    }

    printf("\ni=%d\n",i);

    if (spinor == NULL) {
        return -1;
    }

    return 0;
}

/* spi nor write */
int spi_nor_write(u32 addr, u8 * data, u32 length)
{
    if(NULL != spinor){
	if(NULL != spinor->write){
		return spinor->write(addr, data, length);
	}
    }
    
    return -1;
}

int spi_nor_read(void *src, void *dest, int len)
{
    if(NULL != spinor){
	if(NULL != spinor->read){
		return spinor->read(src, dest, len);
	}
    }
    
    return -1;
}

int spi_nor_erase(u32 addr, u32 length)
{
    if(NULL != spinor){
	if(NULL != spinor->erase){
		return spinor->erase(addr, length);
	}
    }
    
    return -1;
}

#if 0
int spi_nor_test_enable;
static int spi_nor_test(void)
{
    u32 src[128];
    u32 dst[128];
    u32 i;
    u8 id[4];

    if (!spi_nor_test_enable) {
        return TEST_NOTPRESENT;
    }

    PROMPT_RUN_TEST("SPI NOR FLASH");

#if defined(SABRE_AI)
    printf("\nConfigure J3 to 2-3 position when running the test.\n");
    printf("and when test is over configure J3 to 1-2 position.\n");
    printf("Has jumper J3 been properly configured?\n");
    if (!is_input_char('y')) {
        printf("  skipping SPI NOR FLASH test \n");
        return TEST_BYPASSED;
    }
#endif

    spi_init_flash(&imx_spi_nor);

    // Look through table for device data
    spi_nor_query((u32 *) id, spi_nor_flash_type);

    // Do we find the device? If not, return error.
    printf("Get chip id: 0x%x, 0x%x, 0x%x, 0x%x\n", id[0], id[1], id[2], id[3]);

    if (spi_nor_flash_type == AT45DB321D) {
        if ((id[0] == AT45DB321D_id.id0) && (id[1] == AT45DB321D_id.id1)
            && (id[2] == AT45DB321D_id.id2)) {
            printf("SPI nor flash chip AT45DBxx found.\n");
            printf("chip id checking PASS.\n");

        } else {
            printf("chip id checking FAIL.\n");
            return TEST_FAILED;
        }
    } else if (spi_nor_flash_type == M25P32) {
        if ((id[0] == M25P32_id.id0) && (id[1] == M25P32_id.id1)
            && (id[2] == M25P32_id.id2)) {
            printf("SPI nor flash chip M25P32xx found.\n");
            printf("chip id checking PASS.\n");

        } else {
            printf("chip id checking FAIL.\n");
            return TEST_FAILED;
        }
    }

    src[0] = 0x12345678;
    src[1] = 0x22222222;

    for (i = 2; i < 128; i++) {
        src[i] = src[i - 1] + src[i - 2];
    }

    printf("Erase spi-nor flash...\n");
    spi_nor_erase(0, 64 * 1024, spi_nor_flash_type);

    printf("Program spi-nor flash... \n");
    if (spi_nor_write(0, (void *)src, 512, spi_nor_flash_type) == 1) {
        /* Not tested as we need a power cycle */
        printf(" SPI NOR test has been bypassed \n");
        return TEST_FAILED;
    }
    printf("Read back data from spi-nor flash \n");
    memset((void *)dst, 0x0, 512);
    spi_nor_read(0, (void *)dst, 512, spi_nor_flash_type);
    printf("Compare data...\n");

    // compare data...
    for (i = 0; i < 128; i++) {
        if (dst[i] != src[i]) {
            printf("SPI NOR verify failed. \n");
            printf("[0x%x] src: 0x%x, dst: 0x%x\n", i, src[i], dst[i]);
            return TEST_FAILED;
        }
    }
    printf("spi-nor test PASS \n");
    return TEST_PASSED;
}

RUN_TEST("SPI NOR FLASH", spi_nor_test)
#endif
