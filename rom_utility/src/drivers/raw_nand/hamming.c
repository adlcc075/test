/*
 *  encode haming code 
 *
 *  Copyright (c) 2008 by Embedded Alley Solution Inc.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */
#include <io.h>
#include "hamming.h"

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief count number of 1s and return true if they occur even number of times
//!        in the given byte.
//!
//! xor all the bits of data, if even number of 1s in u8, then the result is 0.
//!
//! \param[in]  data // input byte
//!
//! \retval    true, if even number of 1s in data
//! \retval    false, if odd number of 1s in data
//!
////////////////////////////////////////////////////////////////////////////////
static BOOL IsNumOf1sEven(U8 data)
{
    int i, nCountOf1s = 0;

    for (i = 0; i < 8; i++) {
        nCountOf1s ^= ((data & (1 << i)) >> i);
    }
    return !(nCountOf1s);
}

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief calculates parity using Hsiao Code and Hamming code
//!
//! \param[in]  d, given 8 bits integer
//! \param[out] p, pointer to uint8_t for parity
//!
//! \retval    none.
//!
////////////////////////////////////////////////////////////////////////////////
static void CalculateParity(U8 d, U8 * p)
{
    U8 Bit0 = (d & (1 << 0)) ? 1 : 0;
    U8 Bit1 = (d & (1 << 1)) ? 1 : 0;
    U8 Bit2 = (d & (1 << 2)) ? 1 : 0;
    U8 Bit3 = (d & (1 << 3)) ? 1 : 0;
    U8 Bit4 = (d & (1 << 4)) ? 1 : 0;
    U8 Bit5 = (d & (1 << 5)) ? 1 : 0;
    U8 Bit6 = (d & (1 << 6)) ? 1 : 0;
    U8 Bit7 = (d & (1 << 7)) ? 1 : 0;

    *p = 0;

    *p |= ((Bit6 ^ Bit5 ^ Bit3 ^ Bit2) << 0);
    *p |= ((Bit7 ^ Bit5 ^ Bit4 ^ Bit2 ^ Bit1) << 1);
    *p |= ((Bit7 ^ Bit6 ^ Bit5 ^ Bit1 ^ Bit0) << 2);
    *p |= ((Bit7 ^ Bit4 ^ Bit3 ^ Bit0) << 3);
    *p |= ((Bit6 ^ Bit4 ^ Bit3 ^ Bit2 ^ Bit1 ^ Bit0) << 4);
}

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief looks up for a match in syndrome table array.
//!
//! \param[in]  u8Synd given syndrome to match in the table
//! \param[out] pu8BitToFlip pointer to return the index of array that matches
//!             with given syndrome
//!
//! \retval    SUCCESS if a match is found
//! \retval    ERROR_ROM_NAND_DRIVER_FCB_SYNDROME_TABLE_MISMATCH no match found
//!
////////////////////////////////////////////////////////////////////////////////
static int TableLookupSingleErrors(U8 u8Synd, U8 * pu8BitToFlip)
{
    U8 i;
	const U8 au8SyndTable[] = {
    	0x1C, 0x16, 0x13, 0x19, 0x1A, 0x07, 0x15, 0x0E, 0x01, 0x02, 0x04, 0x08, 0x10
	};

    for (i = 0; i < 13; i++) {
        if (au8SyndTable[i] == u8Synd) {
            *pu8BitToFlip = i;
            return TRUE;
        }
    }
    return FALSE;
}

static U8 calculate_parity_22_16(U16 d)
{
    U8 p = 0;

    if (d == 0 || d == 0xFFFF)
        return 0;               /* optimization :) */

    p |= BSEQ(d, 15, 12, 11, 8, 5, 4, 3, 2) << 0;
    p |= BSEQ(d, 13, 12, 11, 10, 9, 7, 3, 1) << 1;
    p |= BSEQ(d, 15, 14, 13, 11, 10, 9, 6, 5) << 2;
    p |= BSEQ(d, 15, 14, 13, 8, 7, 6, 4, 0) << 3;
    p |= BSEQ(d, 12, 9, 8, 7, 6, 2, 1, 0) << 4;
    p |= BSEQ(d, 14, 10, 5, 4, 3, 2, 1, 0) << 5;
    return p;
}

static U8 calculate_parity_13_8(U8 d)
{
    U8 p = 0;

    p |= (B(d, 6) ^ B(d, 5) ^ B(d, 3) ^ B(d, 2)) << 0;
    p |= (B(d, 7) ^ B(d, 5) ^ B(d, 4) ^ B(d, 2) ^ B(d, 1)) << 1;
    p |= (B(d, 7) ^ B(d, 6) ^ B(d, 5) ^ B(d, 1) ^ B(d, 0)) << 2;
    p |= (B(d, 7) ^ B(d, 4) ^ B(d, 3) ^ B(d, 0)) << 3;
    p |= (B(d, 6) ^ B(d, 4) ^ B(d, 3) ^ B(d, 2) ^ B(d, 1) ^ B(d, 0)) << 4;
    return p;
}

int encode_hamming_code_22_16(void *source_block, size_t source_size,
                              void *target_block, size_t target_size)
{
    int i, j, bit_index;
    U16 *src;
    U8 *dst;
    U8 np;
    U8 ecc[NAND_HC_ECC_SIZEOF_PARITY_BLOCK_IN_BYTES];
    U8 data[NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES];

    memset(data, 0, ARRAY_SIZE(data));
    memcpy(data, source_block, source_size);

    src = (U16 *) data;
    dst = (U8 *) target_block;

    /* create THREE copies of source block */
    for (i = 0; i < NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES; i++) {
        dst[i + NAND_HC_ECC_OFFSET_FIRST_DATA_COPY] =
            dst[i + NAND_HC_ECC_OFFSET_SECOND_DATA_COPY] =
            dst[i + NAND_HC_ECC_OFFSET_THIRD_DATA_COPY] = ((U8 *) src)[i];
    }

    /* finally, point to the end of populated data */
    for (bit_index = j = i = 0; j < NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES / sizeof(U16); j++) {
        np = calculate_parity_22_16(src[j]);

        switch (bit_index) {

        case 0:
            ecc[i] = np & 0x3F;
            break;
        case 2:
            ecc[i++] |= (np & 0x03) << 6;
            ecc[i] = (np & 0x3C) >> 2;
            break;
        case 4:
            ecc[i++] |= (np & 0x0F) << 4;
            ecc[i] = (np & 0x30) >> 4;
            break;
        case 6:
            ecc[i++] |= (np & 0x3F) << 2;
            break;
        }
        bit_index = (bit_index + 2) % 8;
    }

    for (i = 0; i < NAND_HC_ECC_SIZEOF_PARITY_BLOCK_IN_BYTES; i++) {
        dst[i + NAND_HC_ECC_OFFSET_FIRST_PARITY_COPY] =
            dst[i + NAND_HC_ECC_OFFSET_SECOND_PARITY_COPY] =
            dst[i + NAND_HC_ECC_OFFSET_THIRD_PARITY_COPY] = ecc[i];
    }

    return 0;
}

int encode_hamming_code_13_8(void *source_block, size_t source_size,
                             void *target_block, size_t target_size)
{
    U8 ecc[NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES / 2];
    U8 data[NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES / 2];
    int i;
    size_t size, left_size = source_size;

    target_block = (U8 *) target_block + BCB_MAGIC_OFFSET;  // 12B offset left

    while (left_size > 0) {
        memset(ecc, 0, ARRAY_SIZE(ecc));
        memset(data, 0, ARRAY_SIZE(data));
        memcpy(data, source_block, ARRAY_SIZE(data));

        /* calculate parity using hamming code */
        size = (left_size < ARRAY_SIZE(data)) ? left_size : ARRAY_SIZE(data);
        for (i = 0; i < size; i++)
            ecc[i] = calculate_parity_13_8(data[i]);

        /* copy data and parity to target */
        memcpy((U8 *) target_block, data, size);
        memcpy((U8 *) target_block + NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES, ecc, size);

        /* calculate pointer and size */
        target_block = (U8 *) target_block + size;
        source_block = (U8 *) source_block + size;
        left_size -= size;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief evaluate FCB block with Hamming Codes
//!
//! This function evaluates FCB Block with Hamming codes and if single bit error
//! occurs then it is fixed, if double error occurs then it returns an error
//!
//! \param[in] pFCB, FCB block
//! \param[in] pParityBlock, block of parity codes, every 6 bits for every 8 bits of 
//!            data in FCB block
//!
//! \retval    SUCCESS, if no error or 1 bit error that is fixed.
//! \retval    ERROR_ROM_NAND_ECC_FAILED, double error occured 
//!            that cannot be fixed.
//!
////////////////////////////////////////////////////////////////////////////////
int HammingCheck(U8 * pFCB, U8 * pParityBlock)
{
    U8 *pu8Data = (U8 *) pFCB;
    U8 P;
    U8 NP;
    int i;
    U8 u8Syndrome, u8BitToFlip;
    int retStatus = TRUE;

    for (i = 0; i < NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES; i++) {
        // Put parity for ith byte in P
        P = pParityBlock[i];

        // calculate new parity out of 8-bit data
        CalculateParity(*pu8Data, &NP);

        // calculate syndrome by XORing 5 parity bits read from NAND and new parity NP just calculated.
        u8Syndrome = NP ^ (P & 0x1F);

        // if syndrome is 0, that means the data is good.
        if (u8Syndrome == 0) {
            // data is good. fetch next 8 bits
            pu8Data++;
            continue;
        }
        // Here we can check for only single and double bit errors. This method does not detect more than
        // two bit errors. If there are more than 2 bit errors they go undetected.
        // Check for double bit errors, which is the case when we have even number of 1s in the syndrome
        if (IsNumOf1sEven(u8Syndrome)) {
            // found a double error, can't fix it, return
            //     printf_PRINTF("Hamming: Found a double bit error in 8-bit data at byte offset %d\n", i);
            return FALSE;
        } else {
            // this is a single bit error and can be fixed
            retStatus = TableLookupSingleErrors(u8Syndrome, &u8BitToFlip);
            if (retStatus != TRUE) {
                retStatus = FALSE;
                return retStatus;
            }

            if (u8BitToFlip < 8) {
                // error is in data bit u8BitToFlip, flip that bit to correct it
                *pu8Data ^= (0x1 << u8BitToFlip);
                // printf_PRINTF("Hamming: Found a single bit error in 8-bit data at byte offset %d bit %d\n", i, u8BitToFlip);
            } else {
                //printf_PRINTF("Hamming: Found a single bit error in parity byte at offset %d bit %d\n", i, u8BitToFlip);
            }
        }
        // fetch next 8 bits
        pu8Data++;
    }

    return retStatus;
}
