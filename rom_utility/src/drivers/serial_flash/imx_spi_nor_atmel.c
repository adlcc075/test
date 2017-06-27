/*
 * Copyright (C) 2010-2012, Freescale Semiconductor, Inc. All Rights Reserved
 * THIS SOURCE CODE IS CONFIDENTIAL AND PROPRIETARY AND MAY NOT
 * BE USED OR DISTRIBUTED WITHOUT THE WRITTEN PERMISSION OF
 * Freescale Semiconductor, Inc.
*/

#include "io.h"
#include "spi/imx_spi.h"
#include "imx_spi_nor_atmel.h"
#include "imx_spi_nor.h"


//#define SPINOR_DEBUG
#ifdef SPINOR_DEBUG
#define DBG(fmt,args...) printf(fmt, ## args)
#else
#define DBG(fmt,args...)
#endif

extern imx_spi_init_func_t spi_init_flash;
extern imx_spi_xfer_func_t spi_xfer_flash;

extern spinor_info_p spinor;

extern u8 g_spi_nor_tx_buf[];
extern u8 g_spi_nor_rx_buf[];

// The remaining elements of imx_spi_nor is filled in the files like mx53.c or mx50.c, etc.
extern struct imx_spi_dev imx_spi_nor;

/* read atmel status register */
static int spi_nor_status_atmel(void)
{
    g_spi_nor_tx_buf[1] = STAT_READ;
    spi_xfer_flash(&imx_spi_nor, g_spi_nor_tx_buf, g_spi_nor_rx_buf, 2 * 8);

    return g_spi_nor_rx_buf[0];
}

/* spi nor write */
int spi_nor_write_atmel(u32 addr, u8 * data, u32 length)
{
    u32 d_addr = (u32) addr;
    u8 *s_buf = (u8 *) data;
    u32 final_addr = 0;
    int page_size = 0, trans_bytes = 0, buf_ptr = 0, bytes_sent = 0, byte_sent_per_iter = 0;
    int page_no = 0, buf_addr = 0, page_off = 0, i = 0, fifo_size = 32;
    int len = length;

    /* Read the status register to get the Page size */
    if (!(spi_nor_status_atmel() & STAT_PG_SZ)) {
        printf("Unsupported Page Size of 528 bytes\n");
        // reprogram the page size to be the 256 or 512 bytess
        g_spi_nor_tx_buf[0] = CONFIG_REG4;
        g_spi_nor_tx_buf[1] = CONFIG_REG3;
        g_spi_nor_tx_buf[2] = CONFIG_REG2;
        g_spi_nor_tx_buf[3] = CONFIG_REG1;

        if (spi_xfer_flash(&imx_spi_nor, g_spi_nor_tx_buf, g_spi_nor_rx_buf, 4 * 8) != 0) {
            printf("Error: %s(): %d\n", __FUNCTION__, __LINE__);
            return -1;
        }

        while (!(spi_nor_status_atmel() & STAT_BUSY)) {
        }

        printf("Reprogrammed the Page Size to be 256/512 bytes\n");
        printf("Please Power Cycle the board for the change to take effect\n");
        return 1;
    }

    if (ATMEL_AT45DB021E == spinor->type)
        page_size = 256;
    else
        page_size = 512;

    /* Due to the way CSPI operates send data less that 4 bytes in a different manner */
    len -= len % 4;

    while (1) {
        page_no = d_addr / page_size;
        page_off = (d_addr % page_size);

        /* Get the offset within the page if address is not page-aligned */
        if (page_off) {
            if (page_no == 0) {
                buf_addr = d_addr;
            } else {
                buf_addr = page_off;
            }

            trans_bytes = page_size - buf_addr;
        } else {
            buf_addr = 0;
            trans_bytes = page_size;
        }

        if (len <= 0)
            break;

        if (trans_bytes > len)
            trans_bytes = len;

        bytes_sent = trans_bytes;

        /* Write the data to the SPI-NOR Buffer first */
        while (trans_bytes > 0) {
            if (ATMEL_AT45DB021E == spinor->type)
                final_addr = (buf_addr & 0xFF);
            else
                final_addr = (buf_addr & 0x3FF);

            i = 0;
            
            g_spi_nor_tx_buf[i++] = final_addr;
            g_spi_nor_tx_buf[i++] = final_addr >> 8;
            g_spi_nor_tx_buf[i++] = final_addr >> 16;
            g_spi_nor_tx_buf[i++] = BUF1_WR;  /*Opcode */

            /* 4 bytes already used for Opcode & address bytes,
               check to ensure we do not overflow the SPI TX buffer */
            if (trans_bytes > (fifo_size - i))
                byte_sent_per_iter = fifo_size;
            else
                byte_sent_per_iter = trans_bytes + i;

            for (; i < byte_sent_per_iter; i += 4) {
                g_spi_nor_tx_buf[i + 3] = s_buf[buf_ptr++];
                g_spi_nor_tx_buf[i + 2] = s_buf[buf_ptr++];
                g_spi_nor_tx_buf[i + 1] = s_buf[buf_ptr++];
                g_spi_nor_tx_buf[i] = s_buf[buf_ptr++];
            }

            spi_xfer_flash(&imx_spi_nor, g_spi_nor_tx_buf, g_spi_nor_rx_buf,
                           byte_sent_per_iter * 8);

            while (!(spi_nor_status_atmel() & STAT_BUSY)) {
            }

            /* Deduct 4 bytes as it is used for Opcode & address bytes */
            trans_bytes -= (byte_sent_per_iter - 4);
            /* Update the destination buffer address */
            buf_addr += (byte_sent_per_iter - 4);
        }

        /* Send the command to write data from the SPI-NOR Buffer to Flash memory */
        if (ATMEL_AT45DB021E == spinor->type)
            final_addr = (page_no & 0x3FF) << 8;
        else
            final_addr = (page_no & 0x1FFF) << 9;

        /* Specify the Page address in Flash where the data should be written to */
        g_spi_nor_tx_buf[0] = final_addr;
        g_spi_nor_tx_buf[1] = final_addr >> 8;
        g_spi_nor_tx_buf[2] = final_addr >> 16;
        g_spi_nor_tx_buf[3] = BUF1_TO_MEM;  /*Opcode */
        if (spi_xfer_flash(&imx_spi_nor, g_spi_nor_tx_buf, g_spi_nor_rx_buf, 4 * 8) != 0) {
            printf("Error: %s(%d): failed\n", __FILE__, __LINE__);
            return -1;
        }

        while (!(spi_nor_status_atmel() & STAT_BUSY)) {
        }

        d_addr += bytes_sent;
        len -= bytes_sent;
    }

    return 0;
}

static int atmel_erase_pbs(u32 d_addr, int pbs)
{
    u8 cmd[3] = { 0x81, 0x50, 0x7c };   //erase page, block, sector

    g_spi_nor_tx_buf[0] = d_addr;
    g_spi_nor_tx_buf[1] = d_addr >> 8;
    g_spi_nor_tx_buf[2] = d_addr >> 16;
    g_spi_nor_tx_buf[3] = cmd[pbs]; /*Opcode */

    spi_xfer_flash(&imx_spi_nor, g_spi_nor_tx_buf, g_spi_nor_rx_buf, 4 * 8);

    while (!(spi_nor_status_atmel() & STAT_BUSY)) {
    }

    return 0;
}

static int atmel_erase_chip(void)
{
    *(u32 *) g_spi_nor_tx_buf = 0xc794809a;

    spi_xfer_flash(&imx_spi_nor, g_spi_nor_tx_buf, g_spi_nor_rx_buf, 4 * 8);

    while (!(spi_nor_status_atmel() & STAT_BUSY)) {
    }

    return 0;
}

/* spi nor erase */
int spi_nor_erase_atmel(u32 addr, u32 length)
{
    u32 page_size = 0, block_size = SZ_4K, sector_size = SZ_64K;
    u32 device_size = SZ_64K * 64;  //4MB;
    u32 d_addr = 0;
    u32 len_left = 0;

    if (ATMEL_AT45DB021E == spinor->type)
        page_size = 256;
    else
        page_size = 512;

    if (length > (device_size - page_size)) {
        /* erase the whole chip */
        atmel_erase_chip();
    } else {
        d_addr = (addr / page_size) * page_size;    //must be page aligned
        len_left = (length / page_size) * page_size;    //must be page aligned

        while (len_left > 0) {
            if ((d_addr % block_size) || (len_left < block_size)) {
                atmel_erase_pbs(d_addr, 0); //erase page
                d_addr += page_size;
                len_left -= page_size;
            } else if ((d_addr % sector_size) || (len_left < sector_size)) {
                atmel_erase_pbs(d_addr, 1); //erase block
                d_addr += block_size;
                len_left -= block_size;
            } else {
                atmel_erase_pbs(d_addr, 2); //erase sector
                d_addr += sector_size;
                len_left -= sector_size;
            }
        }
    }

    return 0;
}

int spi_nor_read_atmel(void *src, void *dest, int len)
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
