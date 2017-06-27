#ifndef HAL_NAND_H
#define HAL_NAND_H

/*==================================================================================================
     Header Name: hal_nand.h
     General Description: Provide some defines for nand IP
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
====================================================================================================
*===================================================================================
                                        INCLUDE FILES
==================================================================================*/
#include <io.h>

/*================================================================================
                                 MACROS
=================================================================================*/
#define DUMMY_BLOCKN_SIZE	0x200

//! This define sets the index where the Auxillary buffer begins.  
#define NAND_AUX_BUFFER_INDEX    16500

/*================================================================================
                                 Global FUNCTION PROTOTYPES
=================================================================================*/
void hal_nand_wait_for_ready(U32 cs);
BOOL hal_nand_send_cmd(U32 cs, U8 cmd);
BOOL hal_nand_send_addr(U32 cs, U8 * addr, U16 bytes);
BOOL hal_nand_read_raw_data(U32 cs, U8 * buf, U16 bytes, BOOL bDDR);
BOOL hal_nand_write_raw_data(U32 cs, U8 * buf, U16 bytes, BOOL bDDR,
                             BOOL rand_enable, U32 rand_type, U32 rand_page);
BOOL hal_nand_write_data_with_ecc(U32 cs, U8 * buf, U16 bytes, BOOL bDDR,
                              BOOL rand_enable, U32 rand_type, U32 rand_page);
BOOL hal_nand_read_data(U32 cs, U8 * buf, U16 bytes, BOOL bDDR, BOOL ecc_enable,
                                     BOOL rand_enable, U32 rand_type, U32 rand_page);

#endif /* HAL_NAND_H */
