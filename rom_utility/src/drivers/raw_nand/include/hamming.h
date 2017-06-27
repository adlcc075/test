#ifndef HAMMING_H
#define HAMMING_H

/*==================================================================================================
     Header Name: hamming.h
     General Description: hamming encode and decode
====================================================================================================

                         Copyright: 2004-2014 FREESCALE, INC.
                   All Rights Reserved. This file contains copyrighted material.
                   Use of this file is restricted by the provisions of a
                   Freescale Software License Agreement, which has either
                   accompanied the delivery of this software in shrink wrap
                   form or been expressly executed between the parties.

Revision History:
                  Modification    Tracking
Author (core ID)      Date         Number     Description of Changes
-----------------  -----------   ----------   ------------------------------------------
Terry Xie           2/Jan/2014                Initial Implementation
-----------------  ------------  ----------   ------------------------------------------

===================================================================================
                                        INCLUDE FILES
==================================================================================*/

/*================================================================================
                          Hash Defines And Macros
=================================================================================*/
#define B(v,n)	(((v) >> (n)) & 0x1)
#define BSEQ(v, a1, a2, a3, a4, a5, a6, a7, a8) \
	(B(v, a1) ^ B(v, a2) ^ B(v, a3) ^ B(v, a4) ^ B(v, a5) ^ B(v, a6) ^ B(v, a7) ^ B(v, a8))

//#ifdef NAND_BURN_FCB_WITH_READ_RETRY
//#define NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES        (1024)
//#else
#define NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES        (512)
//#endif

//! Offset to first copy of FCB in a NAND page
#define NAND_HC_ECC_OFFSET_DATA_COPY            (12)    // make sure this value is divisible by 4 for buffers to be word aligned

//! Offset to first copy of Parity block in a NAND page
#define NAND_HC_ECC_OFFSET_PARITY_COPY          (NAND_HC_ECC_OFFSET_DATA_COPY+NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES)

//! Size of a parity block in bytes for all 16-bit data blocks present inside one 512 byte NCB block.
#define NAND_HC_ECC_SIZEOF_PARITY_BLOCK_IN_BYTES      ((((512*8)/16)*6)/8)

//! Offset to the copies of NCB in a NAND page
#define NAND_HC_ECC_OFFSET_FIRST_DATA_COPY            (0)
#define NAND_HC_ECC_OFFSET_SECOND_DATA_COPY           (NAND_HC_ECC_OFFSET_FIRST_DATA_COPY+NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES)
#define NAND_HC_ECC_OFFSET_THIRD_DATA_COPY            (NAND_HC_ECC_OFFSET_SECOND_DATA_COPY+NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES)
#define NAND_HC_ECC_OFFSET_FIRST_PARITY_COPY          (NAND_HC_ECC_OFFSET_THIRD_DATA_COPY+NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES)
#define NAND_HC_ECC_OFFSET_SECOND_PARITY_COPY         (NAND_HC_ECC_OFFSET_FIRST_PARITY_COPY+NAND_HC_ECC_SIZEOF_PARITY_BLOCK_IN_BYTES)
#define NAND_HC_ECC_OFFSET_THIRD_PARITY_COPY          (NAND_HC_ECC_OFFSET_SECOND_PARITY_COPY+NAND_HC_ECC_SIZEOF_PARITY_BLOCK_IN_BYTES)

#define BCB_MAGIC_OFFSET 12

/*=================================================================================
                                 Global FUNCTION PROTOTYPES
=================================================================================*/
int encode_hamming_code_13_8(void *source_block, U32 source_size,
                                    void *target_block, U32 target_size);
int encode_hamming_code_22_16(void *source_block, U32 source_size,
                                     void *target_block, U32 target_size);
int HammingCheck(U8 * pFCB, U8 * pParityBlock);

#endif
