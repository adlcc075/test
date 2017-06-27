#ifndef HAL_DMA_H
#define HAL_DMA_H

/*=================================================================================

    Module Name:  hal_dma.h

    General Description: Header file Limited IROM NFC Driver.

===================================================================================
                               Freescale Confidential Proprietary
                        Template ID and version: IL93-TMP-01-0106  Version 1.20
                        (c) Copyright Freescale, All Rights Reserved

                     Copyright: 2004-2014 FREESCALE, INC.
                   All Rights Reserved. This file contains copyrighted material.
                   Use of this file is restricted by the provisions of a
                   Freescale Software License Agreement, which has either
                   accompanied the delivery of this software in shrink wrap
                   form or been expressly executed between the parties.

Revision History:
                            Modification     Tracking
Author (core ID)                Date          Number     Description of Changes
-------------------------   ------------    ----------   --------------------------
Terry Xie	                18-NOV-2014                 Clean up file
Fareed Mohammed             24-Mar-2010    ENGR00121599 Removed unused stuff.
Fareed Mohammed             09-Mar-2010    ENGR00118666 Added boot support for toggle mode DDR NAND

Portability: Portable to other compilers or platforms.
===================================================================================
                                        INCLUDE FILES
==================================================================================*/
#include "regsapbh.h"
#include "regsgpmi.h"
#include "regs.h"
#include "io.h"

/*================================================================================
                          Hash Defines And Macros
=================================================================================*/
//! should equate to ~12msec.
#define MAX_TRANSACTION_TIMEOUT             12000   //!< Maximum time for DMA is 12msec

//! value is used to mask the correct NAND channels.
#define NAND0_APBH_CH            0  //!< Define starting bit for NANDs in APBH.

//! \brief APBH DMA Macro for Wait4Ready command.
//!
//! Transfer one Word to PIO.
//! Wait for DMA to complete before starting next DMA descriptor in chain.
//! Wait for Ready before starting next DMA descriptor in chain.
//! Don't lock the nand while waiting for Ready to go high.
//! Another descriptor follows this one in the chain.
//! This DMA has no transfer.
#define NAND_DMA_WAIT4RDY_CMD \
    (BF_APBH_CHn_CMD_CMDWORDS(1) | \
     BF_APBH_CHn_CMD_WAIT4ENDCMD(1) | \
     BF_APBH_CHn_CMD_NANDWAIT4READY(1) | \
     BF_APBH_CHn_CMD_NANDLOCK(0) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER))

//! \brief GPMI PIO DMA Macro for Wait4Ready command.
//!
//! Wait for Ready before sending IRQ interrupt.
//! Use 8 bit word length (doesn't really matter since no transfer).
//! Watch u32ChipSelect.
#define NAND_DMA_WAIT4RDY_PIO(u32ChipSelect) \
    (BV_FLD(GPMI_CTRL0, COMMAND_MODE, WAIT_FOR_READY) | \
     BF_GPMI_CTRL0_WORD_LENGTH(BV_GPMI_CTRL0_WORD_LENGTH__8_BIT) | \
     BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA) | \
     BF_GPMI_CTRL0_CS(u32ChipSelect))

//! \brief APBH DMA Macro for Transmit Data command.
//!
//! Transfer TransferSize bytes with DMA.
//! Transfer one Word to PIO.
//! Wait for DMA to complete before starting next DMA descriptor in chain.
//! Lock the NAND while waiting for this DMA chain to complete.
//! Decrement semaphore if this is the last part of the chain.
//! Another descriptor follows this one in the chain.
//! This DMA is a read from System Memory - write to device.
                                      // TGT_3700, TGT_CHIP and others
#define NAND_DMA_TXDATA_CMD(TransferSize,Semaphore,CommandWords,Wait4End,DmaType) \
    (BF_APBH_CHn_CMD_XFER_COUNT(TransferSize) | \
     BF_APBH_CHn_CMD_CMDWORDS(CommandWords) | \
	 BF_APBH_CHn_CMD_HALTONTERMINATE(1) | \
     BF_APBH_CHn_CMD_WAIT4ENDCMD(Wait4End) | \
     BF_APBH_CHn_CMD_NANDLOCK(1) | \
     BF_APBH_CHn_CMD_SEMAPHORE(Semaphore) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, DmaType))

//! \brief GPMI PIO DMA Macro for Transmit Data command.
//!
//! Setup transfer as a write.
//! Transfer NumBitsInWord bits per DMA cycle.
//! Lock CS during this transaction.
//! Select the appropriate chip.
//! Address lines need to specify Data transfer (0b00)
//! Transfer TransferSize - NumBitsInWord values.
#define NAND_DMA_TXDATA_PIO(u32ChipSelect,NumBitsInWord,TransferSize) \
    (BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE) | \
     BF_GPMI_CTRL0_WORD_LENGTH(NumBitsInWord) | \
     BF_GPMI_CTRL0_LOCK_CS(1) | \
     BF_GPMI_CTRL0_CS(u32ChipSelect) | \
     BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA) | \
     BF_GPMI_CTRL0_XFER_COUNT(TransferSize))

//! \brief APBH DMA Macro for Sense command.
//!
//! Transfer no Bytes with DMA.
//! Transfer no Words to PIO.
//! Don't lock the NAND while waiting for Ready to go high.
//! Decrement semaphore if this is the last part of the chain.
//! Another descriptor follows this one in the chain.
#define NAND_DMA_SENSE_CMD(SenseSemaphore) \
    (BF_APBH_CHn_CMD_CMDWORDS(0) | \
     BF_APBH_CHn_CMD_SEMAPHORE(SenseSemaphore) | \
     BF_APBH_CHn_CMD_NANDLOCK(0) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, DMA_SENSE))

//! \brief APBH DMA Macro for Read Data command.
//!
//! Receive TransferSize bytes with DMA.
//! Transfer one Word to PIO.
//! Wait for DMA to complete before starting next DMA descriptor in chain.
//! Decrement semaphore if this is the last part of the chain.
//! Unlock the NAND after this DMA chain completes.
//! Another descriptor follows this one in the chain.
//! This DMA is a write to System Memory - read from device.

#define NAND_DMA_RX_CMD_NOECC(TransferSize,CmdWords,Semaphore) \
    (BF_APBH_CHn_CMD_XFER_COUNT(TransferSize) | \
     BF_APBH_CHn_CMD_CMDWORDS(CmdWords) | \
     BF_APBH_CHn_CMD_WAIT4ENDCMD(1) | \
     BF_APBH_CHn_CMD_SEMAPHORE(Semaphore) | \
     BF_APBH_CHn_CMD_NANDLOCK(0) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, DMA_WRITE))

//! \brief APBH DMA Macro for Read Data command with ECC.
//!
//! Receive TransferSize bytes with DMA.
//! Transfer one Word to PIO.
//! Wait for DMA to complete before starting next DMA descriptor in chain.
//! Decrement semaphore if this is the last part of the chain.
//! Unlock the NAND after this DMA chain completes.
//! Another descriptor follows this one in the chain.
//! No DMA transfer here; the ECC8 block becomes the bus master and
//! performs the memory writes itself instead of the DMA.
#define NAND_DMA_RX_CMD_ECC(TransferSize,Semaphore) \
    (BF_APBH_CHn_CMD_XFER_COUNT(TransferSize) | \
     BF_APBH_CHn_CMD_CMDWORDS(6) | \
     BF_APBH_CHn_CMD_WAIT4ENDCMD(1) | \
     BF_APBH_CHn_CMD_SEMAPHORE(Semaphore) | \
     BF_APBH_CHn_CMD_NANDLOCK(1) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER))

//! \brief APBH DMA Macro for Recieve Data with no ECC command.
//!
//! Receive TransferSize bytes with DMA but no ECC.
//! Transfer one Word to PIO.
//! Wait for DMA to complete before starting next DMA descriptor in chain.
//! Decrement semaphore if this is the last part of the chain.
//! Unlock the NAND after this DMA chain completes.
//! Another descriptor follows this one in the chain.
//! This DMA is a write to System Memory - read from device.
#define NAND_DMA_RX_NO_ECC_CMD(TransferSize,Semaphore) \
    (BF_APBH_CHn_CMD_XFER_COUNT(TransferSize) | \
     BF_APBH_CHn_CMD_CMDWORDS(1) | \
     BF_APBH_CHn_CMD_WAIT4ENDCMD(1) | \
     BF_APBH_CHn_CMD_SEMAPHORE(Semaphore) | \
     BF_APBH_CHn_CMD_NANDLOCK(0) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, DMA_WRITE))

//! \brief GPMI PIO DMA Macro for Receive command.
//!
//! Setup transfer as a READ.
//! Transfer NumBitsInWord bits per DMA cycle.
//! Select the appropriate chip.
//! Address lines need to specify Data transfer (0b00)
//! Transfer TransferSize - NumBitsInWord values.
#define NAND_DMA_RX_PIO(u32ChipSelect,NumBitsInWord,TransferSize) \
    (BV_FLD(GPMI_CTRL0, COMMAND_MODE, READ) | \
     BF_GPMI_CTRL0_WORD_LENGTH(NumBitsInWord) | \
     BF_GPMI_CTRL0_CS(u32ChipSelect) | \
     BF_GPMI_CTRL0_LOCK_CS(0) | \
     BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA) | \
     BF_GPMI_CTRL0_XFER_COUNT(TransferSize))

//! \brief APBH DMA Macro for sending NAND Command sequence.
//!
//! Transmit TransferSize bytes to DMA.
//! Transfer one Word to PIO.
//! Wait for DMA to complete before starting next DMA descriptor in chain.
//! Decrement semaphore if this is the last part of the chain.
//! Lock the NAND until the next chain.
//! Another descriptor follows this one in the chain.
//! This DMA is a read from System Memory - write to device.
#define NAND_DMA_COMMAND_CMD(TransferSize,Semaphore,NandLock,CmdWords) \
    (BF_APBH_CHn_CMD_XFER_COUNT(TransferSize) | \
     BF_APBH_CHn_CMD_CMDWORDS(CmdWords) | \
     BF_APBH_CHn_CMD_WAIT4ENDCMD(1) | \
     BF_APBH_CHn_CMD_SEMAPHORE(Semaphore) | \
     BF_APBH_CHn_CMD_NANDLOCK(NandLock) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, DMA_READ))

//! \brief GPMI PIO DMA Macro when sending a command.
//!
//! Setup transfer as a WRITE.
//! Transfer NumBitsInWord bits per DMA cycle.
//! Lock CS during and after this transaction.
//! Select the appropriate chip.
//! Address lines need to specify Command transfer (0b01)
//! Increment the Address lines if AddrIncr is set.
//! Transfer TransferSize - NumBitsInWord values.
#define NAND_DMA_COMMAND_PIO(u32ChipSelect,TransferSize,AddrInc,AssertCS) \
    (BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE) | \
    BF_GPMI_CTRL0_WORD_LENGTH(BV_GPMI_CTRL0_WORD_LENGTH__8_BIT) | \
    BF_GPMI_CTRL0_LOCK_CS(AssertCS) | \
    BF_GPMI_CTRL0_CS(u32ChipSelect) | \
    BV_FLD(GPMI_CTRL0, ADDRESS, NAND_CLE) | \
    BF_GPMI_CTRL0_ADDRESS_INCREMENT(AddrInc) | \
    BF_GPMI_CTRL0_XFER_COUNT(TransferSize))

//! \brief GPMI PIO DMA Macro for disabling ECC during this write.
#define NAND_DMA_ECC_PIO(EnableDisable) \
    (BW_GPMI_ECCCTRL_ENABLE_ECC(EnableDisable))

//! \brief APBH DMA Macro for Sending NAND Address sequence.
//!
//! Setup transfer as a WRITE.
//! Transfer NumBitsInWord bits per DMA cycle.
//! Lock CS during and after this transaction.
//! Select the appropriate chip.
//! Address lines need to specify Address transfer (0b10)
//! Transfer TransferSize - NumBitsInWord values.
#define NAND_DMA_ADDRESS_PIO(u32ChipSelect,TransferSize) \
    (BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE) | \
     BF_GPMI_CTRL0_WORD_LENGTH(BV_GPMI_CTRL0_WORD_LENGTH__8_BIT) | \
     BF_GPMI_CTRL0_LOCK_CS(1) | \
     BF_GPMI_CTRL0_CS(u32ChipSelect) | \
     BV_FLD(GPMI_CTRL0, ADDRESS, NAND_ALE) | \
     BF_GPMI_CTRL0_XFER_COUNT(TransferSize))

//! \brief APBH DMA Macro for NAND Compare sequence
//!
//! Transfer TransferSize Word to PIO.
//! Wait for DMA to complete before starting next DMA descriptor in chain.
//! Lock the NAND until the next chain.
//! Another descriptor follows this one in the chain.
//! This DMA has no transfer.
#define NAND_DMA_COMPARE_CMD(TransferSize) \
    (BF_APBH_CHn_CMD_CMDWORDS(TransferSize) | \
     BF_APBH_CHn_CMD_WAIT4ENDCMD(0) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BF_APBH_CHn_CMD_NANDLOCK(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER))

//! \brief GPMI PIO DMA Macro for NAND Compare sequence
//!
//! Setup transfer as a Read and Compare.
//! Transfer 8 bits per DMA cycle.
//! Lock CS during and after this transaction.
//! Select the appropriate chip.
//! Address lines need to specify Data transfer (0b00)
//! Transfer TransferSize - 8 Bit values.
#define NAND_DMA_COMPARE_PIO(u32ChipSelect,TransferSize) \
    (BV_FLD(GPMI_CTRL0, COMMAND_MODE, READ_AND_COMPARE) | \
     BF_GPMI_CTRL0_WORD_LENGTH(BV_GPMI_CTRL0_WORD_LENGTH__8_BIT) | \
     BF_GPMI_CTRL0_CS(u32ChipSelect) | \
     BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA) | \
     BF_GPMI_CTRL0_XFER_COUNT(TransferSize))

//! \brief APBH DMA Macro for NAND Dummy transfer sequence
//!
//! Dummy Transfer.
//! Wait for DMA to complete before starting next DMA descriptor in chain.
//! Lock the NAND until the next chain.
//! Another descriptor follows this one in the chain.
//! This DMA has no transfer.
#define NAND_DMA_DUMMY_TRANSFER \
    (BF_APBH_CHn_CMD_CMDWORDS(0) | \
     BF_APBH_CHn_CMD_WAIT4ENDCMD(0) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BF_APBH_CHn_CMD_NANDLOCK(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER))

//! \brief GPMI PIO DMA Macro sequence for ECC decode.
//!
//! Setup READ transfer ECC Control register.
//! Setup for ECC Decode, 4 Bit.
//! Enable the ECC block
//! The ECC Buffer Mask determines which fields are corrected.
#define NAND_DMA_ECC_CTRL_PIO(EccBufferMask, decode_encode_size) \
    (BW_GPMI_ECCCTRL_ECC_CMD(decode_encode_size) | \
     BW_GPMI_ECCCTRL_ENABLE_ECC(BV_GPMI_ECCCTRL_ENABLE_ECC__ENABLE) | \
     BW_GPMI_ECCCTRL_BUFFER_MASK(EccBufferMask) )

//! \brief APBH DMA Macro for Disabling the ECC block sequence
//!
//! Disable ECC Block Transfer.
//! Wait for DMA to complete before starting next DMA descriptor in chain.
//! Lock the NAND until the next chain.
//! Another descriptor follows this one in the chain.
//! This DMA has no transfer.
#define NAND_DMA_DISABLE_ECC_TRANSFER \
    (BF_APBH_CHn_CMD_CMDWORDS(3) | \
     BF_APBH_CHn_CMD_WAIT4ENDCMD(1) | \
     BF_APBH_CHn_CMD_NANDWAIT4READY(1) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BF_APBH_CHn_CMD_NANDLOCK(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER))

//! \brief GPMI PIO DMA Macro sequence for disabling ECC block
//!
//! Setup transfer as a READ.
//! Release CS during and after this transaction.
//! Select the appropriate chip.
//! Address lines need to specify Data transfer
//! Transfer NO data.
#define NAND_DMA_DISABLE_ECC_PIO(u32ChipSelect) \
    (BV_FLD(GPMI_CTRL0, COMMAND_MODE, WAIT_FOR_READY) | \
    BF_GPMI_CTRL0_WORD_LENGTH(BV_GPMI_CTRL0_WORD_LENGTH__8_BIT) | \
    BF_GPMI_CTRL0_LOCK_CS(0) | \
    BF_GPMI_CTRL0_CS(u32ChipSelect) | \
    BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA) | \
    BF_GPMI_CTRL0_ADDRESS_INCREMENT(0) | \
    BF_GPMI_CTRL0_XFER_COUNT(0))

//! \brief APBH DMA Macro for Removing the NAND Lock in APBH.
//!
//! Remove NAND Lock.
//! Wait for DMA to complete before starting next DMA descriptor in chain.
//! Unlock the NAND to allow another NAND to run.
//! Another descriptor follows this one in the chain.
//! This DMA has no transfer.
#define NAND_DMA_REMOVE_NAND_LOCK \
    (BF_APBH_CHn_CMD_CMDWORDS(0) | \
     BF_APBH_CHn_CMD_WAIT4ENDCMD(1) | \
     BF_APBH_CHn_CMD_NANDWAIT4READY(0) | \
     BF_APBH_CHn_CMD_NANDLOCK(0) | \
     BF_APBH_CHn_CMD_CHAIN(1) | \
     BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER))

#define DECR_SEMAPHORE  1       //!< Decrement DMA semaphore this time.
#define NAND_LOCK       1       //!< Lock the NAND to prevent contention.
#define ASSERT_CS       1       //!< Assert the Chip Select during this operation.
#define ECC_ENCODE      1       //!< Perform an ECC Encode during this operation.
#define ECC_DECODE      0       //!< Perform an ECC Decode during this operation.
#define ECC_4_BIT       0       //!< Perform a 4 bit ECC operation.
#define ECC_8_BIT       1       //!< Perform an 8 bit ECC operation.

/*=================================================================================
                                 STRUCTURES AND OTHER TYPEDEFS
=================================================================================*/
typedef struct _dma_cmd {
    struct _dma_cmd *pNxt;
    hw_apbh_chn_cmd_t cmd;
    void *pBuf;
    hw_gpmi_ctrl0_t ctrl;
    hw_gpmi_compare_t cmp;
} dma_cmd_t;

///////////////////////////////////////////////////////////////////
//! \name APBH DMA Structure Definitions
//@{
//////////////////////////////////////////////////////////////////

//! Define the APBH DMA structure without GPMI transfers.
typedef struct _apbh_dma_t {
    struct _apbh_dma_t *nxt;
    hw_apbh_chn_cmd_t cmd;
    void *bar;
} apbh_dma_t;

//! Define the APBH DMA structure with 1 GPMI Parameter word writes.
typedef struct _apbh_dma_gpmi1_t {
    struct _apbh_dma_gpmi1_t *nxt;
    hw_apbh_chn_cmd_t cmd;
    void *bar;
    union {
        struct {
            hw_gpmi_ctrl0_t gpmi_ctrl0;
        } apbh_dma_gpmi1_ctrl;
        reg32_t pio[1];
    } apbh_dma_gpmi1_u;
} apbh_dma_gpmi1_t;

//! Define the APBH DMA structure with 2 GPMI Parameter word writes.
typedef struct _apbh_dma_gpmi2_t {
    struct _apbh_dma_gpmi2_t *nxt;
    hw_apbh_chn_cmd_t cmd;
    void *bar;
    union {
        struct {
            hw_gpmi_ctrl0_t gpmi_ctrl0;
            hw_gpmi_compare_t gpmi_compare;
        } apbh_dma_gpmi2_ctrlcompare;
        reg32_t pio[2];
    } apbh_dma_gpmi2_u;
} apbh_dma_gpmi2_t;

//! Define the APBH DMA structure with 3 GPMI Parameter word writes.
typedef struct _apbh_dma_gpmi3_t {
    struct _apbh_dma_gpmi1_t *nxt;
    hw_apbh_chn_cmd_t cmd;
    void *bar;
    union {
        struct {
            hw_gpmi_ctrl0_t gpmi_ctrl0;
            hw_gpmi_compare_t gpmi_compare;
            hw_gpmi_eccctrl_t gpmi_eccctrl;
        } apbh_dma_gpmi3_ctrl;
        reg32_t pio[3];
    } apbh_dma_gpmi3_u;
} apbh_dma_gpmi3_t;

//! Define the APBH DMA structure with 4 GPMI Parameter word writes.
typedef struct _apbh_dma_gpmi4_t {
    struct _apbh_dma_gpmi1_t *nxt;
    hw_apbh_chn_cmd_t cmd;
    void *bar;
    union {
        struct {
            hw_gpmi_ctrl0_t gpmi_ctrl0;
            hw_gpmi_compare_t gpmi_compare;
            hw_gpmi_eccctrl_t gpmi_eccctrl;
            hw_gpmi_ecccount_t gpmi_ecccount;
        } apbh_dma_gpmi4_reg;
        reg32_t pio[4];
    } apbh_dma_gpmi4_u;
} apbh_dma_gpmi4_t;

//! Define the APBH DMA structure with 5 GPMI Parameter word writes.
typedef struct _apbh_dma_gpmi5_t {
    struct _apbh_dma_gpmi1_t *nxt;
    hw_apbh_chn_cmd_t cmd;
    void *bar;
    union {
        struct {
            hw_gpmi_ctrl0_t gpmi_ctrl0;
            hw_gpmi_compare_t gpmi_compare;
            hw_gpmi_eccctrl_t gpmi_eccctrl;
            hw_gpmi_ecccount_t gpmi_ecccount;
            hw_gpmi_payload_t gpmi_payload;
        } apbh_dma_gpmi5_reg;
        reg32_t pio[5];
    } apbh_dma_gpmi5_u;
} apbh_dma_gpmi5_t;

//! Define the APBH DMA structure with 6 GPMI Parameter word writes.
typedef struct _apbh_dma_gpmi6_t {
    struct _apbh_dma_gpmi1_t *nxt;
    hw_apbh_chn_cmd_t cmd;
    void *bar;
    union {
        struct {
            hw_gpmi_ctrl0_t gpmi_ctrl0;
            hw_gpmi_compare_t gpmi_compare;
            hw_gpmi_eccctrl_t gpmi_eccctrl;
            hw_gpmi_ecccount_t gpmi_ecccount;
            hw_gpmi_payload_t gpmi_payload;
            hw_gpmi_auxiliary_t gpmi_auxiliary;
        } apbh_dma_gpmi6_ctrl;
        reg32_t pio[6];
    } apbh_dma_gpmi6_u;
} apbh_dma_gpmi6_t;

/*=================================================================================
                                 Global FUNCTION PROTOTYPES
=================================================================================*/
void dma_start(void *pDmaChain, U32 cs);
BOOL dma_wait(U32 u32uSecTimeout, U32 cs);

#endif /* HAL_DMA_H */