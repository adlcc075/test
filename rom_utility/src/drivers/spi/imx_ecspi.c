/*
 * Copyright (C) 2010-2012, Freescale Semiconductor, Inc. All Rights Reserved
 * THIS SOURCE CODE IS CONFIDENTIAL AND PROPRIETARY AND MAY NOT
 * BE USED OR DISTRIBUTED WITHOUT THE WRITTEN PERMISSION OF
 * Freescale Semiconductor, Inc.
*/

#include "io.h"
#include "timer/timer.h"
#include "imx_spi.h"

/*!
 * Initialization function for a spi slave device. It must be called BEFORE
 * any spi operations. The SPI module will be -disabled- after this call.
 */
#if 0 // Not needed since we'll hack ROM's init routine.
int imx_ecspi_init(struct imx_spi_dev *dev)
{
    unsigned int clk_src = get_peri_clock(dev->base);
    unsigned int pre_div = 1, post_div = 1, i, reg_ctrl, reg_config;
    struct spi_v2_3_reg *reg = (struct spi_v2_3_reg *)dev->reg;

    if (dev->freq == 0) {
        printf("Error: desired clock is 0\n");
        return -1;
    }
    // iomux config
    //io_cfg_spi(dev);

    reg_ctrl = readl(dev->base + 0x8);
    // reset the spi
    writel(0, dev->base + 0x8);
    writel(reg_ctrl | 0x1, dev->base + 0x8);

    // control register setup
    if (clk_src > dev->freq) {
        pre_div = clk_src / dev->freq;
        if (pre_div > 16) {
            post_div = pre_div / 16;
            pre_div = 15;
        }
        if (post_div != 0) {
            for (i = 0; i < 16; i++) {
                if ((1 << i) >= post_div)
                    break;
            }
            if (i == 16) {
                printf("Error: no divider can meet the freq: %d\n", dev->freq);
                return -1;
            }
            post_div = i;
        }
    }
    SPIDBG("pre_div = %d, post_div=%d\n", pre_div, post_div);
    reg_ctrl = (reg_ctrl & ~(3 << 18)) | dev->ss << 18;
    reg_ctrl = (reg_ctrl & ~(0xF << 12)) | pre_div << 12;
    reg_ctrl = (reg_ctrl & ~(0xF << 8)) | post_div << 8;
    reg_ctrl |= 1 << (dev->ss + 4); // always set to master mode !!!!
    reg_ctrl &= ~0x1;           // disable spi

    reg_config = readl(dev->base + 0xC);
    // configuration register setup
    reg_config = (reg_config & ~(1 << ((dev->ss + 12)))) | (dev->ss_pol << (dev->ss + 12));
    reg_config = (reg_config & ~(1 << ((dev->ss + 20)))) | (dev->in_sctl << (dev->ss + 20));
    reg_config = (reg_config & ~(1 << ((dev->ss + 16)))) | (dev->in_dctl << (dev->ss + 16));
    reg_config = (reg_config & ~(1 << ((dev->ss + 8)))) | (dev->ssctl << (dev->ss + 8));
    reg_config = (reg_config & ~(1 << ((dev->ss + 4)))) | (dev->sclkpol << (dev->ss + 4));
    reg_config = (reg_config & ~(1 << ((dev->ss + 0)))) | (dev->sclkpha << (dev->ss + 0));

    SPIDBG("reg_ctrl = 0x%x\n", reg_ctrl);
    writel(reg_ctrl, dev->base + 0x8);
    SPIDBG("reg_config = 0x%x\n", reg_config);
    writel(reg_config, dev->base + 0xC);
    // save config register and control register
    reg->cfg_reg = reg_config;
    reg->ctrl_reg = reg_ctrl;

    // clear interrupt reg
    writel(0, dev->base + 0x10);
    writel(3 << 6, dev->base + 0x18);

    return 0;
}
#endif

/*!
 * This function should only be called after the imx_spi_init_xxx().
 * It sets up the spi module according to the initialized value and then
 * enables the SPI module. This function is called by the xfer function.
 *
 * Note: If one wants to change the SPI parameters such as clock, the
 *       imx_spi_init_xxx() needs to be called again.
 */
static void imx_ecspi_start(struct imx_spi_dev *dev, struct spi_v2_3_reg *reg, int len)
{
    if (reg->ctrl_reg == 0) {
        printf("Error: spi(base=0x%x) has not been initialized yet\n", dev->base);
        return;
    }
    // iomux config
    // io_cfg_spi(dev);	// Not needed since we hacked the ROM's init routine.
    reg->ctrl_reg = (reg->ctrl_reg & ~0xFFF00000) | ((len - 1) << 20);
    writel(reg->ctrl_reg | 0x1, dev->base + 0x8);
    writel(reg->cfg_reg, dev->base + 0xC);
    SPIDBG("ctrl_reg=0x%x, cfg_reg=0x%x\n", readl(dev->base + 0x8), readl(dev->base + 0xC));
}

/*!
 * Stop the SPI module that the slave device is connected to.
 */
static void imx_ecspi_stop(struct imx_spi_dev *dev)
{
    writel(0, dev->base + 0x8);
}

/*!
 * Transfer up to burst_bytes bytes data via spi. The amount of data
 * is the sum of both the tx and rx.
 * After this call, the SPI module that the slave is connected to will
 * be -disabled- again.
 */
int imx_ecspi_xfer(struct imx_spi_dev *dev, // spi device pointer
                   unsigned char *tx_buf,   // tx buffer (has to be 4-byte aligned)
                   unsigned char *rx_buf,   // rx buffer (has to be 4-byte aligned)
                   int burst_bits   // total number of bits in one burst (or xfer)
    )
{
    int val = SPI_RETRY_TIMES;
    unsigned int *p_buf;
    unsigned int reg;
    int len, ret_val = 0;
    int burst_bytes = burst_bits / 8;

    /* Account for rounding of non-byte aligned burst sizes */
    if ((burst_bits % 8) != 0)
        burst_bytes++;
    if (burst_bytes > dev->fifo_sz) {
        printf("Error: maximum burst size is 0x%x bytes, asking 0x%x\n", dev->fifo_sz, burst_bytes);
        return -1;
    }

    imx_ecspi_start(dev, dev->reg, burst_bits);

    // move data to the tx fifo
    for (p_buf = (unsigned int *)tx_buf, len = burst_bytes; len > 0; p_buf++, len -= 4) {
        writel(*p_buf, dev->base + 0x4);
    }
    reg = readl(dev->base + 0x8);
    reg |= (1 << 2);            // set xch bit
    SPIDBG("control reg = 0x%08x\n", reg);
    writel(reg, dev->base + 0x8);

    // poll on the TC bit (transfer complete)
    while ((val-- > 0) && (readl(dev->base + 0x18) & (1 << 7)) == 0) {
        if (dev->us_delay != 0) {
            GPT_reset_delay(dev->us_delay/30);
        }
    }

    // clear the TC bit
    writel(3 << 6, dev->base + 0x18);
    if (val == 0) {
        printf("Error: re-tried %d times without response. Give up\n", SPI_RETRY_TIMES);
        ret_val = -1;
        goto error;
    }
    // move data in the rx buf
    for (p_buf = (unsigned int *)rx_buf, len = burst_bytes; len > 0; p_buf++, len -= 4) {
        *p_buf = readl(dev->base + 0x0);
    }
  error:
    imx_ecspi_stop(dev);
    return ret_val;
}
