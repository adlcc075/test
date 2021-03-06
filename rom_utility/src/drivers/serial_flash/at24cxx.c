/*
 * Copyright (C) 2011-2012, Freescale Semiconductor, Inc. All Rights Reserved
 * THIS SOURCE CODE IS CONFIDENTIAL AND PROPRIETARY AND MAY NOT
 * BE USED OR DISTRIBUTED WITHOUT THE WRITTEN PERMISSION OF
 * Freescale Semiconductor, Inc.
*/

#include "io.h"
#include "i2c/imx_i2c.h"
#include "platform.h"

#define AT24Cx_I2C_ID 0x50      // ROM hardcoded the eeprom device address as 0x50
#define AT24Cx_PAGE_SZ	128

#define BYTE_SWAP(v)	((((v) >> 8) & 0xff) | (((v) & 0xff) << 8))

int at24cx_read(u32 i2c_base_addr, u32 addr, u8 * buf, u32 sz)
{
    struct imx_i2c_request rq;

    rq.dev_addr = AT24Cx_I2C_ID;
    rq.reg_addr = addr;
    rq.reg_addr_sz = 2;
    rq.buffer = buf;

    while (sz > 0) {
        rq.buffer_sz = sz > AT24Cx_PAGE_SZ ? AT24Cx_PAGE_SZ : sz;
        rq.reg_addr = BYTE_SWAP(rq.reg_addr);
        if (0 != i2c_xfer(i2c_base_addr, &rq, I2C_READ)) {
            return -1;
        }

        sz -= rq.buffer_sz;
        rq.buffer += rq.buffer_sz;
        rq.reg_addr += rq.buffer_sz;
    }

    return 0;
}

int at24cx_write(u32 i2c_base_addr, u32 addr, u8 * buf, u32 sz)
{
    struct imx_i2c_request rq;

    rq.dev_addr = AT24Cx_I2C_ID;
    rq.reg_addr = addr;
    rq.reg_addr_sz = 2;
    rq.buffer = buf;

    while (sz > 0) {
        rq.buffer_sz = sz > AT24Cx_PAGE_SZ ? AT24Cx_PAGE_SZ : sz;
        rq.reg_addr = BYTE_SWAP(rq.reg_addr);
        if (0 != i2c_xfer(i2c_base_addr, &rq, I2C_WRITE)) {
            return -1;
        }

        sz -= rq.buffer_sz;
        rq.buffer += rq.buffer_sz;
        rq.reg_addr += rq.buffer_sz;
    }
    return 0;
}

#if 0
static int i2c_eeprom_at24cxx_test(void)
{
    unsigned char buf1[4], buf2[4];

    if (!eeprom_test_enable) {
        return TEST_NOTPRESENT;
    }

    PROMPT_RUN_TEST("EEPROM test");

    i2c_init(AT24Cx_I2C_BASE, 170000);

#if (!defined(MX6SL) && !defined(MX6SLX) && defined(EVB))
    /*Set iomux and daisy chain for eeprom test */
    reg32_write(IOMUXC_SW_MUX_CTL_PAD_EIM_D17, ALT6 | 0x10);
    reg32_write(IOMUXC_I2C3_IPP_SCL_IN_SELECT_INPUT, 0x00);
    reg32_write(IOMUXC_SW_PAD_CTL_PAD_EIM_D17, 0x1b8b0);

    reg32_write(IOMUXC_SW_MUX_CTL_PAD_EIM_D18, ALT6 | 0x10);
    reg32_write(IOMUXC_I2C3_IPP_SDA_IN_SELECT_INPUT, 0x00);
    reg32_write(IOMUXC_SW_PAD_CTL_PAD_EIM_D18, 0x1b8b0);
#endif

    buf1[0] = 'F';              //0x89;
    buf1[1] = 'S';              //0x11;
    buf1[2] = 'L';

    at24cx_write(AT24Cx_I2C_BASE, 0, buf1);

    buf2[0] = 0;
    buf2[1] = 0;
    buf2[2] = 0;
    at24cx_read(AT24Cx_I2C_BASE, 0, buf2);

#if (!defined(MX6SL) && defined(EVB))
    /*Restore iomux and daisy chain setting */
    i2c_init(AT24Cx_I2C_BASE, 170000);
#endif

    if ((buf2[0] != buf1[0]) || (buf2[1] != buf1[1]) || (buf2[2] != buf1[2])) {
        printf("i2c eeprom test fail.\n");
        printf("please make sure EEPROM is mounted on board\n");
        return TEST_FAILED;
    } else {
        printf("i2c eeprom test pass.\n");
        return TEST_PASSED;
    }
}

RUN_TEST("EEPROM", i2c_eeprom_at24cxx_test)
#endif
