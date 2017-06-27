/*
 * Copyright (c) 2011-2012, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*-------------------------- Definitions ------------------------------*/

#ifndef __USDHC_H__
#define __USDHC_H__

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAIL
#define FAIL 1
#endif

/*-------------------------- Enumerations ------------------------------*/

//! @brief boot part
typedef enum {
    EMMC_PART_USER,
    EMMC_PART_BOOT1,
    EMMC_PART_BOOT2
} emmc_part_e;

//! @brief eMMC bus width
typedef enum {
    EMMC_BOOT_SDR1,
    EMMC_BOOT_SDR4,
    EMMC_BOOT_SDR8,
    EMMC_BOOT_DDR4,
    EMMC_BOOT_DDR8
} emmc_bus_width_e;


/*------------------------- Function Defines ------------------------------*/

/*!
 * @brief Read data from card
 *
 * @param instance     Instance number of the uSDHC module.
 * @param dst_ptr      Data destination pointer
 * @param length       Data length to be read
 * @param offset       Data reading offset
 *
 * @return             0 if successful; 1 otherwise
 */
extern int card_data_read(uint32_t instance, int *dst_ptr, int length, uint32_t offset);

/*!
 * @brief Write data to card
 *
 * @param instance     Instance number of the uSDHC module.
 * @param src_ptr      Data source pointer
 * @param length       Data length to be writen
 * @param offset       Data writing offset
 *
 * @return             0 if successful; 1 otherwise
 */
extern int card_data_write(uint32_t instance, int *src_ptr, int length, int offset);

/*!
 * @brief Set boot partition
 *
 * @param   instance     Instance number of the uSDHC module.
  * @param  part     boot partition index.
 *
 * @return 0 if successful; non-zero otherwise
 */
extern int mmc_set_boot_partition(uint32_t instance, emmc_part_e part);

/*!
 * @brief Configure bus for boot
 *
 * @param   instance     Instance number of the uSDHC module.
 * @param  width     boot bus configuration.
 *
 * @return 0 if successful; non-zero otherwise
 */
extern int mmc_set_boot_bus_width(uint32_t instance, emmc_bus_width_e width);

/*!
 * @brief Set boot config field of ECSD register
 *
 * @param   instance     Instance number of the uSDHC module.
 * @param  part     boot partition index.
 * @param  ack     enable/disable acknowledge.
 *
 * @return 0 if successful; non-zero otherwise
 */
extern int mmc_set_boot_config(uint32_t instance, emmc_part_e part, bool ack);

/*!
 * @brief enable boot partition access
 *
 * @param   instance     Instance number of the uSDHC module.
  * @param  part     boot partition index.
 *
 * @return 0 if successful; non-zero otherwise
 */
extern int mmc_set_bp_access(uint32_t instance, emmc_part_e part);

/*!
 * @brief Dump usdhc controller register
 *
 */
extern void host_registers_dump(uint32_t instance);

/*!
 * @brief Dump mmc extern CSD register
 *
 */
extern void mmc_dump_ecsd(uint32_t instance);

/*!
 * @brief Dump mmc CSD register
 *
 */
extern void mmc_dump_csd(usdhc_port_e port);

/*!
 * @brief Dump mmc OCR register
 *
 */
extern void mmc_dump_ocr(usdhc_port_e port);

/*!
 * @brief Dump CID register
 *
 */
extern void card_dump_cid(usdhc_port_e port);

/*!
 * @brief Get mmc card version
 *
 */
extern void mmc_get_version_simple(uint32_t instance);

#endif
