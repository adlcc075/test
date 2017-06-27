/*
 * Copyright (C) 2010-2012, Freescale Semiconductor, Inc. All Rights Reserved
 * THIS SOURCE CODE IS CONFIDENTIAL AND PROPRIETARY AND MAY NOT
 * BE USED OR DISTRIBUTED WITHOUT THE WRITTEN PERMISSION OF
 * Freescale Semiconductor, Inc.
*/

#include "io.h"
#include "spi/imx_spi.h"
#include "imx_spi_nor_numonyx.h"

//#define SPINOR_DEBUG
#ifdef SPINOR_DEBUG
#define DBG(fmt,args...) printf(fmt, ## args)
#else
#define DBG(fmt,args...)
#endif

extern imx_spi_init_func_t spi_init_flash;
extern imx_spi_xfer_func_t spi_xfer_flash;

extern u8 g_spi_nor_tx_buf[];
extern u8 g_spi_nor_rx_buf[];

// The remaining elements of imx_spi_nor is filled in the files like mx53.c or mx50.c, etc.
extern struct imx_spi_dev imx_spi_nor;

/* read numonyx status register */
static int spi_nor_status_numonyx(void)
{
    g_spi_nor_tx_buf[1] = RDSR;
    spi_xfer_flash(&imx_spi_nor, g_spi_nor_tx_buf, g_spi_nor_rx_buf, 2 * 8);

    return g_spi_nor_rx_buf[0];
}

/*write enable*/
static int spi_nor_wren_numonyx(void)
{
    g_spi_nor_tx_buf[0] = WREN;
    spi_xfer_flash(&imx_spi_nor, g_spi_nor_tx_buf, g_spi_nor_rx_buf, 1 * 8);

    return 0;
}

/* spi nor write */
int spi_nor_write_numonyx(u32 addr, u8 * data, u32 length)
{
    u32 d_addr = (u32) addr;
    u8 *s_buf = (u8 *) data;
    u32 final_addr = 0;
    int page_size = 256, trans_bytes = 0, buf_ptr = 0, bytes_sent = 0, byte_sent_per_iter = 0;
    int /*page_no = 0,*/ buf_addr = 0, page_off = 0, i = 0, j = 0, k = 0, fifo_size = 32;
    int remain_len = 0, len = length;

    /* Due to the way CSPI operates send data less that 4 bytes in a different manner */
    remain_len = len % 4;

    if (remain_len)
        len -= remain_len;

    while (1) {
        //page_no = d_addr / page_size;
        page_off = (d_addr % page_size);

        /* Get the offset within the page if address is not page-aligned */
        if (page_off) {
            trans_bytes = page_size - page_off;
        } else {
            buf_addr = 0;
            trans_bytes = page_size;
        }

        if (len <= 0)
            break;

        if (trans_bytes > len)
            trans_bytes = len;

        bytes_sent = trans_bytes;

        final_addr = d_addr;

        while (trans_bytes > 0) {
            /*Write enable before every tranfer. */
            spi_nor_wren_numonyx();
            /*Refresh the address to write. */
            g_spi_nor_tx_buf[0] = final_addr;
            g_spi_nor_tx_buf[1] = final_addr >> 8;
            g_spi_nor_tx_buf[2] = final_addr >> 16;
            g_spi_nor_tx_buf[3] = PP;   /*Opcode */

            /* 4 bytes already used for Opcode & address bytes,
               check to ensure we do not overflow the SPI TX buffer */
            if (trans_bytes > (fifo_size - 4))
                byte_sent_per_iter = fifo_size;
            else
                byte_sent_per_iter = trans_bytes + 4;

            for (i = 4; i < byte_sent_per_iter; i += 4) {
                g_spi_nor_tx_buf[i + 3] = s_buf[buf_ptr++];
                g_spi_nor_tx_buf[i + 2] = s_buf[buf_ptr++];
                g_spi_nor_tx_buf[i + 1] = s_buf[buf_ptr++];
                g_spi_nor_tx_buf[i] = s_buf[buf_ptr++];
            }

            spi_xfer_flash(&imx_spi_nor, g_spi_nor_tx_buf, g_spi_nor_rx_buf,
                           byte_sent_per_iter * 8);

            while ((spi_nor_status_numonyx() & RDSR_BUSY)) {
            }

            /* Deduct 4 bytes as it is used for Opcode & address bytes */
            trans_bytes -= (byte_sent_per_iter - 4);
            /* Update the destination buffer address */
            final_addr += (byte_sent_per_iter - 4);
        }

        d_addr += bytes_sent;
        len -= bytes_sent;
    }

    if (remain_len) {
        /*Write enable before every tranfer. */
        spi_nor_wren_numonyx();

        buf_ptr += remain_len;

        /* Write the remaining data bytes first */
        for (i = 0; i < remain_len; i++) {
            g_spi_nor_tx_buf[i] = s_buf[buf_ptr--];
        }

        /* Write the address bytes next in the same word as the data byte from the next byte */
        for (j = i, k = 0; j < 4; j++, k++) {
            g_spi_nor_tx_buf[j] = final_addr >> (k * 8);
        }

        /* Write the remaining address bytes in the next word */
        j = 0;
        final_addr = (buf_addr & 0x3FF);

        for (j = 0; k < 3; j++, k++) {
            g_spi_nor_tx_buf[j] = final_addr >> (k * 8);
        }

        /* Finally the Opcode to write the data to the buffer */
        g_spi_nor_tx_buf[j] = PP;   /*Opcode */

        if (spi_xfer_flash(&imx_spi_nor, g_spi_nor_tx_buf, g_spi_nor_rx_buf, (remain_len + 4) * 8)
            != 0) {
            printf("Error: %s(%d): failed\n", __FILE__, __LINE__);
            return -1;
        }

        while ((spi_nor_status_numonyx() & RDSR_BUSY)) {
        }
    }

    printf("\nwrite command is successful.");
    return 0;
}

static int numonyx_erase_sector(u32 d_addr)
{
    spi_nor_wren_numonyx();     /*write enable */

    g_spi_nor_tx_buf[0] = d_addr;
    g_spi_nor_tx_buf[1] = d_addr >> 8;
    g_spi_nor_tx_buf[2] = d_addr >> 16;
    g_spi_nor_tx_buf[3] = SE;   /*Opcode */

    spi_xfer_flash(&imx_spi_nor, g_spi_nor_tx_buf, g_spi_nor_rx_buf, 4 * 8);

    while ((spi_nor_status_numonyx() & RDSR_BUSY)) {
    }

    return 0;
}

static int numonyx_erase_chip(void)
{
    spi_nor_wren_numonyx();

    g_spi_nor_tx_buf[0] = BE;

    spi_xfer_flash(&imx_spi_nor, g_spi_nor_tx_buf, g_spi_nor_rx_buf, 1 * 8);

    while ((spi_nor_status_numonyx() & RDSR_BUSY)) {
    }

    return 0;
}

/* spi nor erase */
int spi_nor_erase_numonyx(u32 addr, u32 length)
{
    u32 sector_size = SZ_64K;
    u32 device_size = SZ_64K * 64;  //4MB;
    int len_left = length;

    if (length > (device_size - sector_size)) {
        /* erase the whole chip */
        numonyx_erase_chip();
    } else {
        while (len_left > 0) {
            numonyx_erase_sector(addr); //erase sector
            addr += sector_size;
            len_left -= sector_size;
        }
    }
    printf("\nerase is successful.");
    return 0;
}

int spi_nor_read_numonyx(void *src, void *dest, int len)
{
    unsigned int *cmd = (unsigned int *)g_spi_nor_tx_buf;
    unsigned int max_rx_sz = 32 - 4;    // max rx bytes per burst
    unsigned char *d_buf = (unsigned char *)dest;
    unsigned char *s_buf;
    int i;

    DBG("%s(from flash=%p to ram=%p len=0x%x)\n", __FUNCTION__, src, dest, len);

    if (len == 0)
        return 0;

    *cmd = (READ << 24) | ((unsigned int)src & 0x00FFFFFF);

    while (1) {
        if (len == 0) {
            return 0;
        }

        if (len < max_rx_sz) {
            DBG("last read len=0x%x\n", len);

            // deal with the last read
            if (spi_xfer_flash(&imx_spi_nor, g_spi_nor_tx_buf, g_spi_nor_rx_buf, (len + 4) * 8) !=
                0) {
                printf("Error: %s(%d): failed\n", __FILE__, __LINE__);
                return TRANS_FAIL;
            }

            s_buf = g_spi_nor_rx_buf + 4;   // throw away 4 bytes (5th received bytes is real)
            // now adjust the endianness
            for (i = len; i >= 0; i -= 4, s_buf += 4) {
                if (i < 4) {
                    if (i == 1) {
                        *d_buf = s_buf[0];
                    } else if (i == 2) {
                        *d_buf++ = s_buf[1];
                        *d_buf++ = s_buf[0];
                    } else if (i == 3) {
                        *d_buf++ = s_buf[2];
                        *d_buf++ = s_buf[1];
                        *d_buf++ = s_buf[0];
                    }

                    return 0;
                }
                // copy 4 bytes
                *d_buf++ = s_buf[3];
                *d_buf++ = s_buf[2];
                *d_buf++ = s_buf[1];
                *d_buf++ = s_buf[0];
            }
        }
        // now grab max_rx_sz data (+4 is needed due to 4-throw away bytes
        if (spi_xfer_flash(&imx_spi_nor, g_spi_nor_tx_buf, g_spi_nor_rx_buf, (max_rx_sz + 4) * 8) !=
            0) {
            printf("Error: %s(%d): failed\n", __FILE__, __LINE__);
            return TRANS_FAIL;
        }

        s_buf = g_spi_nor_rx_buf + 4;   // throw away 4 bytes (5th received bytes is real)
        // now adjust the endianness
        for (i = 0; i < max_rx_sz; i += 4, s_buf += 4) {
            *d_buf++ = s_buf[3];
            *d_buf++ = s_buf[2];
            *d_buf++ = s_buf[1];
            *d_buf++ = s_buf[0];
        }

        *cmd += max_rx_sz;      // increase # of bytes in NOR address as cmd == g_tx_buf
        len -= max_rx_sz;       // # of bytes left

        //printf("d_buf=%p, g_rx_buf=%p, len=0x%x\n", d_buf, g_spi_nor_rx_buf, len);
    }

}
