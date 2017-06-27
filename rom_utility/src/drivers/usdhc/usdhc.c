/*
 * Copyright (c) 2012, Freescale Semiconductor, Inc.
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

#include "platform.h"
#include "io.h"
#include "debug.h"
#include "usdhc.h"
#include "usdhc_private.h"
#include "usdhc_host.h"
#include "registers/regsusdhc.h"

extern int get_usdhc_port(uint32_t);

/* Global Variables */
usdhc_inst_t usdhc_device[USDHC_NUMBER_PORTS] = {
    {
     1,                         //RCA
     0,                         //addressing mode
     1,                         //status
     }
    ,

    {
     1,                         //RCA
     0,                         //addressing mode
     1,                         //status
     }
    ,

    {
     1,                         //RCA
     0,                         //addressing mode
     1,                         //status
     }
    ,

    {
     1,                         //RCA
     0,                         //addressing mode
     1,                         //status
     }
    ,
};

static uint32_t cid_data_g[USDHC_NUMBER_PORTS][4];

/********************************************* Static Function ******************************************/


/********************************************* Global Function ******************************************/

#if 0
int card_get_csd(int base_address)
{

    command_t cmd;
    command_response_t response;
    int status = FAIL;
    int card_address = (card_rca << RCA_SHIFT);

    /* Configure CMD9 for MMC/SD card */
    mxcmci_cmd_config(&cmd, CMD9, card_address, READ, RESPONSE_136, DATA_PRESENT_NONE, TRUE, FALSE);

    if (host_send_cmd(base_address, &cmd) == SUCCESS) {
        response.format = RESPONSE_136;
        host_read_response(base_address, &response);

        csd.csd0 = response.cmd_rsp0;
        csd.csd1 = response.cmd_rsp1;
        csd.csd2 = response.cmd_rsp2;
        csd.csd3 = response.cmd_rsp3;

        status = SUCCESS;
    }

    return status;
}
#endif

/*!
 * @brief Build up command
 *
 * @param cmd      IPointer of command to be build up.
 * @param index    Command index.
 * @param argument Argument of the command.
 * @param transfer Command transfer type - Read, Write or SD command.
 * @param format   Command response format
 * @param data     0 - no data present, 1 - data present.
 * @param src      0 - no CRC check, 1 - do CRC check
 * @param cmdindex 0 - no check on command index, 1 - Check comamnd index
 */
void card_cmd_config(command_t * cmd, int index, int argument, xfer_type_t transfer,
                     response_format_t format, data_present_select data,
                     crc_check_enable crc, cmdindex_check_enable cmdindex)
{
    cmd->command = index;
    cmd->arg = argument;
    cmd->data_transfer = transfer;
    cmd->response_format = format;
    cmd->data_present = data;
    cmd->crc_check = crc;
    cmd->cmdindex_check = cmdindex;
    cmd->dma_enable = FALSE;
    cmd->block_count_enable_check = FALSE;
    cmd->multi_single_block = SINGLE;
    cmd->acmd12_enable = FALSE;
    cmd->ddren = FALSE;

    /* Multi Block R/W Setting */
    if ((CMD18 == index) || (CMD25 == index)) {
        cmd->block_count_enable_check = TRUE;
        cmd->multi_single_block = MULTIPLE;
        cmd->acmd12_enable = TRUE;
    }
}

/*!
 * @brief Get Card CID
 *
 * @param instance     Instance number of the uSDHC module.
 *
 * @return             0 if successful; 1 otherwise
 */
int card_get_cid(uint32_t instance)
{
    command_t cmd;
    int status = FAIL, port = USDHC_NUMBER_PORTS;
    command_response_t response;

    /* Configure CMD2 */
    card_cmd_config(&cmd, CMD2, NO_ARG, READ, RESPONSE_136, DATA_PRESENT_NONE, TRUE, FALSE);

    DEBUG_LOG("Send CMD2.\n");

    /* Send CMD2 */
    if (host_send_cmd(instance, &cmd) == SUCCESS) {
        response.format = RESPONSE_136;
        host_read_response(instance, &response);

        /* No Need to Save CID */

        port = get_usdhc_port(instance);
        if (USDHC_NUMBER_PORTS == port) {
            DEBUG_LOG("get usdhc port failure\n");
        } else {
            cid_data_g[port][0] = response.cmd_rsp0;
            cid_data_g[port][1] = response.cmd_rsp1;
            cid_data_g[port][2] = response.cmd_rsp2;
            cid_data_g[port][3] = response.cmd_rsp3;
        }

        status = SUCCESS;
    }

    return status;
}

/*!
 * @brief Toggle the card between the standby and transfer states
 *
 * @param instance     Instance number of the uSDHC module.
 *
 * @return             0 if successful; 1 otherwise
 */
int card_enter_trans(uint32_t instance)
{
    command_t cmd;
    int card_address, port, status = FAIL;

    /* Get port from base address */
    port = get_usdhc_port(instance);

    /* Get RCA */
    card_address = usdhc_device[port].rca << RCA_SHIFT;

    /* Configure CMD7 */
    card_cmd_config(&cmd, CMD7, card_address, READ, RESPONSE_48_CHECK_BUSY,
                    DATA_PRESENT_NONE, TRUE, TRUE);

    DEBUG_LOG("Send CMD7.\n");

    /* Send CMD7 */
    if (host_send_cmd(instance, &cmd) == SUCCESS) {
        /* Check if the card in TRAN state */
        if (card_trans_status(instance) == SUCCESS) {
            status = SUCCESS;
        }
    }

    return status;
}

/*!
 * @brief Addressed card send its status register
 *
 * @param instance     Instance number of the uSDHC module.
 *
 * @return             0 if successful; 1 otherwise
 */
int card_trans_status(uint32_t instance)
{
    command_t cmd;
    command_response_t response;
    int card_state, card_address, port, status = FAIL;

    /* Get Port */
    port = get_usdhc_port(instance);

    /* Get RCA */
    card_address = usdhc_device[port].rca << RCA_SHIFT;

    /* Configure CMD13 */
    card_cmd_config(&cmd, CMD13, card_address, READ, RESPONSE_48, DATA_PRESENT_NONE, TRUE, TRUE);

    DEBUG_LOG("Send CMD13.\n");

    /* Send CMD13 */
    if (host_send_cmd(instance, &cmd) == SUCCESS) {
        /* Get Response */
        response.format = RESPONSE_48;
        host_read_response(instance, &response);

        /* Read card state from response */
        card_state = CURR_CARD_STATE(response.cmd_rsp0);
        if (card_state == TRAN) {
            status = SUCCESS;
        } else {
            DEBUG_LOG("unexpected card status: %d\n", card_state);
        }
    } else {
        DEBUG_LOG("send CMD13 failure\n");
    }

    return status;
}

/*!
 * @brief Set block length (in bytes) for read and write
 *
 * @param instance     Instance number of the uSDHC module.
 * @param len          Block length to be set
 *
 * @return             0 if successful; 1 otherwise
 */
int card_set_blklen(uint32_t instance, int len)
{
    command_t cmd;
    int status = FAIL;

    /* Configure CMD16 */
    card_cmd_config(&cmd, CMD16, len, READ, RESPONSE_48, DATA_PRESENT_NONE, TRUE, TRUE);

    DEBUG_LOG("Send CMD16.\n");

    /* Send CMD16 */
    if (host_send_cmd(instance, &cmd) == SUCCESS) {
        status = SUCCESS;
    }

    return status;
}

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
int card_data_read(uint32_t instance, int *dst_ptr, int length, uint32_t offset)
{
    int port, sector;
    command_t cmd;

    /* Get uSDHC port according to instance */
    port = get_usdhc_port(instance);
    if (port == USDHC_NUMBER_PORTS) {
        DEBUG_LOG("Base address: 0x%x not in address table.\n", instance);
        return FAIL;
    }

    DEBUG_LOG("card_data_read: Read 0x%x bytes from SD%d offset 0x%x to 0x%x.\n",
                 length, port + 1, offset, (int)dst_ptr);

    /* Get sector number */
    /* For PIO mode, not neccesary */
    sector = length / BLK_LEN;

    if ((length % BLK_LEN) != 0) {
        sector++;
    }

    /* Offset should be sectors */
    if (usdhc_device[port].addr_mode == SECT_MODE) {
        offset = offset / BLK_LEN;
    }

    /* Set block length to card */
    if (card_set_blklen(instance, BLK_LEN) == FAIL) {
        DEBUG_LOG("Fail to set block length to card in reading sector %d.\n", offset / BLK_LEN);
        return FAIL;
    }

    /* Clear Rx FIFO */
    host_clear_fifo(instance);

    /* Configure block length/number and watermark */
    host_cfg_block(instance, BLK_LEN, sector, ESDHC_BLKATTR_WML_BLOCK);

    /* Use CMD18 for multi-block read */
    card_cmd_config(&cmd, CMD18, offset, READ, RESPONSE_48, DATA_PRESENT, TRUE, TRUE);

    DEBUG_LOG("card_data_read: Send CMD18.\n");

    /* Send CMD18 */
    if (host_send_cmd(instance, &cmd) == FAIL) {
        DEBUG_LOG("Fail to send CMD18.\n");
        return FAIL;
    } else {
        /* In polling IO mode, manually read data from Rx FIFO */
        DEBUG_LOG("Non-DMA mode, read data from FIFO.\n");

        if (host_data_read(instance, dst_ptr, length, ESDHC_BLKATTR_WML_BLOCK) == FAIL) {
            DEBUG_LOG("Fail to read data from card.\n");
            return FAIL;
        }
    }

    DEBUG_LOG("card_data_read: Data read successful.\n");

    return SUCCESS;
}

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
int card_data_write(uint32_t instance, int *src_ptr, int length, int offset)
{
    int port, sector;
    command_t cmd;

    /* Get uSDHC port according to base address */
    port = get_usdhc_port(instance);
    if (port == USDHC_NUMBER_PORTS) {
        DEBUG_LOG("Base address: 0x%x not in address table.\n", instance);
        return FAIL;
    }

    DEBUG_LOG("card_data_write: write 0x%x bytes to SD%d offset 0x%x from 0x%x.\n",
                 length, port + 1, offset, (int)src_ptr);

    /* Get sector number */
    /* For PIO mode, not neccesary */
    sector = length / BLK_LEN;

    if ((length % BLK_LEN) != 0) {
        sector++;
    }

    /* Offset should be sectors */
    if (usdhc_device[port].addr_mode == SECT_MODE) {
        offset = offset / BLK_LEN;
    }

    /* Set block length to card */
    if (card_set_blklen(instance, BLK_LEN) == FAIL) {
        DEBUG_LOG("Fail to set block length to card in writing sector %d.\n", offset / BLK_LEN);
        return FAIL;
    }

    /* Configure block length/number and watermark */
    host_cfg_block(instance, BLK_LEN, sector, ESDHC_BLKATTR_WML_BLOCK << ESDHC_WML_WRITE_SHIFT);

    /* Use CMD25 for multi-block write */
    card_cmd_config(&cmd, CMD25, offset, WRITE, RESPONSE_48, DATA_PRESENT, TRUE, TRUE);

    DEBUG_LOG("card_data_write: Send CMD25.\n");

    /* Send CMD25 */
    if (host_send_cmd(instance, &cmd) == FAIL) {
        DEBUG_LOG("Fail to send CMD25.\n");
        return FAIL;
    } else {
        DEBUG_LOG("Non-DMA mode, write to FIFO.\n");

        if (host_data_write(instance, src_ptr, length, ESDHC_BLKATTR_WML_BLOCK) == FAIL) {
            DEBUG_LOG("Fail to write data to card.\n");
            return FAIL;
        }

        DEBUG_LOG("card_data_write: Data written successful.\n");
    }

    return SUCCESS;
}

/*!
 * @brief Get card status
 *
 * @param instance     Instance number of the uSDHC module.
 * @param result       Card status
 *
 * @return             0 if successful; 1 otherwise
 */
int card_xfer_result(uint32_t instance, int *result)
{
    int idx = get_usdhc_port(instance);

    if (idx == USDHC_NUMBER_PORTS) {
        DEBUG_LOG("Base address: 0x%x not in address table.\n", instance);
        return FAIL;
    }

    *result = usdhc_device[idx].status;

    return SUCCESS;
}

/*!
 * @brief Wait for the transfer complete. It covers the interrupt mode, DMA mode and PIO mode
 *
 * @param instance     Instance number of the uSDHC module.
 *
 * @return             0 if successful; 1 otherwise
 */
int card_wait_xfer_done(uint32_t instance)
{
    int usdhc_status = 0;
    int timeout = 0x40000000;

    while (timeout--) {
        card_xfer_result(instance, &usdhc_status);
        if (usdhc_status == 1)
            return SUCCESS;
    }

     return FAIL;
}

/*!
 * @brief Dump Card CID
 *
 * @param port     usdhc port number.
 */
void card_dump_cid(usdhc_port_e port)
{
    printf("\n");
    printf("CID: 0x%x%x%x%x\n", cid_data_g[port][3], cid_data_g[port][2],
                                cid_data_g[port][1], cid_data_g[port][0]);
}

