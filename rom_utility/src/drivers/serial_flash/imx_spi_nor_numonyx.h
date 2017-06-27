/*
 * Copyright (C) 2010-2012, Freescale Semiconductor, Inc. All Rights Reserved
 * THIS SOURCE CODE IS CONFIDENTIAL AND PROPRIETARY AND MAY NOT
 * BE USED OR DISTRIBUTED WITHOUT THE WRITTEN PERMISSION OF
 * Freescale Semiconductor, Inc.
*/

/*!
 * @file        imx_spi_nor_numonyx.h
 * @brief       header file for Numonyx SPI NOR flash
 *
 * @ingroup     diag_driver
 */

#ifndef _IMX_SPI_NOR_NUMONYX_H_
#define _IMX_SPI_NOR_NUMONYX_H_

#define WREN        0x06        // write enable. 1 byte tx cmd
#define WRDI        0x04        // write disable. 1 byte tx cmd
#define RDID        0x9F        // read JEDEC ID. tx: 1 byte cmd; rx: 3 byte ID
#define RDSR        0x05        // read status register 1 byte tx cmd + 1 byte rx status
#define WRSR        0x01        // Write status register. 1 byte tx cmd + 1 byte tx value
#define READ        0x03        // tx: 1 byte cmd + 3 byte addr; rx: variable bytes
#define FAST_READ   0x0B        // tx: 1 byte cmd + 3 byte addr + 1 byte dummy; rx: variable bytes
#define PP	    0x02            // page program. all tx: 1 cmd + 3 addr + 1 data
#define SE   	    0xD8        // sector erase. 1 byte cmd + 3 byte addr
#define BE	    0xC7            // bulk erase.   1 byte cmd
#define DP	    0xB9            //Deep power-down. 1 byte cmd.
#define RES	    0xAB            //Release from Deep power-down. 1 byte cmd.

#define RDSR_BUSY       (1 << 0)    // 1=write-in-progress (default 0)
#define RDSR_WEL        (1 << 1)    // 1=write enable (default 0)
#define RDSR_BP0        (1 << 2)    // block write prot level (default 1)
#define RDSR_BP1        (1 << 3)    // block write prot level (default 1)
#define RDSR_BP2        (1 << 4)    // block write prot level (default 1)
#define RDSR_SRWD       (1 << 7)    // Status Reg Write Disable.

#define SZ_64K      0x10000
#define SZ_32K      0x8000
#define SZ_4K       0x1000
#define TRANS_FAIL  -1

#endif // _IMX_SPI_NOR_NUMONYX_H_
