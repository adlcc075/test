#ifndef ROM_NAND_BOOT_H
#define ROM_NAND_BOOT_H

/*=================================================================================

    Module Name:  rom_nand_boot.h

    General Description: Header file Limited IROM NAND Driver.

===================================================================================
                               Freescale Confidential Proprietary
                        Template ID and version: IL93-TMP-01-0106  Version 1.20
                        (c) Copyright Freescale, All Rights Reserved

                                Copyright: 2007-2014 FREESCALE, INC.
                   All Rights Reserved. This file contains copyrighted material.
                   Use of this file is restricted by the provisions of a
                   Freescale Software License Agreement, which has either
                   accompanied the delivery of this software in shrink wrap
                   form or been expressly executed between the parties.

Revision History:
                            Modification     Tracking
Author (core ID)                Date          Number     Description of Changes
----------------------   ------------    ----------   	--------------------------
Terry Xie                 2/DEC/2014                    Transplant to ROM utility

Portability: Portable to other compilers or platforms.

===================================================================================
                                        INCLUDE FILES
==================================================================================*/
#include "nand_base.h"
#include "platform.h"
/*=================================================================================
=
                                 LOCAL FUNCTION PROTOTYPES
=================================================================================*/

/*================================================================================
                          Hash Defines And Macros
=================================================================================*/
#define FCB_RSVD_NUM						(15)

#define FCB_FINGERPRINT                    	(0x20424346)
#define FCB_VERSION                    		(0x01000000)

#define RAND_SHIFT (0)
#define RAND_MASK  (0xF <<RAND_SHIFT)
#define FCB_CHECK_SHIFT (4)
#define FCB_CHECK_MASK  (0xF <<FCB_CHECK_SHIFT)
#define READ_RETRY_SHIFT (0x8)
#define READ_RETRY_MASK  (0xF <<READ_RETRY_SHIFT)

#define SET(rand, fcb_check, read_retry) \
			((rand << RAND_SHIFT) \
			| (fcb_check << FCB_CHECK_SHIFT) \
			| (read_retry << READ_RETRY_SHIFT))
#define GET(name, v) 	((v & name##_MASK) >> (name##_SHIFT))

// ! sets max read retry sequence length, for FCB struct use
#define MAX_READ_RETRY_SEQ_LEN (732)

/*=================================================================================
                                 STRUCTURES AND OTHER TYPEDEFS
=================================================================================*/
typedef struct _FCBStruct_t {
    U32 Reserved;               //!< First fingerprint in first byte.
    U32 FingerPrint;            //!< 2nd fingerprint at byte 4.
    U32 Version;                //!< 3rd fingerprint at byte 8.

    NAND_Timing_t m_NANDTiming; //!< Optimum timing parameters for Tas, Tds, Tdh in nsec.
    U32 data_page_sz;           //!< 2048 for 2K pages, 4096 for 4K pages.
    U32 total_page_sz;          //!< 2112 for 2K pages, 4314 for 4K pages.
    U32 pages_per_block;        //!< Number of 2K sections per block.
    U32 NumberOfNANDs;          //!< Total Number of NANDs - not used by ROM.
    U32 TotalInternalDie;       //!< Number of separate chips in this NAND.
    U32 CellType;               //!< MLC or SLC.
    U32 blockn_ecc_level;       //!< Type of ECC, can be one of BCH-0-20
    U32 block0_sz;          	//!< Number of bytes for Block0 - BCH
    U32 blockn_sz;          	//!< Block size in bytes for all blocks other than Block0 - BCH
    U32 block0_ecc_level;       //!< Ecc level for Block 0 - BCH
    U32 meta_sz;          		//!< Metadata size - BCH
    U32 blockn_num_per_page;    //!< Number of blocks per page for ROM use - BCH
    U32 EccBlockNEccLevel_sdk;   //!< Type of ECC, can be one of BCH-0-20
    U32 block0_sz_sdk;       	//!< Number of bytes for Block0 - BCH
    U32 blockn_sz_sdk;       	//!< Block size in bytes for all blocks other than Block0 - BCH
    U32 EccBlock0EccLevel_sdk;   //!< Ecc level for Block 0 - BCH
    U32 blockn_num_per_page_sdk; //!< Number of blocks per page for _sdk use - BCH
    U32 meta_sz_sdk;       		//!< Metadata size - BCH
    U32 EraseThreshold;         //!< To set into BCH_MODE register.
    U32 BootPatch;              //!< 0 for normal boot and 1 to load patch starting next to FCB.
    U32 PatchSectors;           //!< Size of patch in sectors.
    U32 Firmware1_startingSector;   //!< Firmware image starts on this sector.
    U32 Firmware2_startingSector;   //!< Secondary FW Image starting Sector.
    U32 SectorsInFirmware1;     //!< Number of sectors in firmware image.
    U32 SectorsInFirmware2;     //!< Number of sector in secondary FW image.
    U32 DBBTSearchAreaStartAddress; //!< Page address where dbbt search area begins
    U32 BadBlockMarkerByte;     //!< Byte in page data that have manufacturer marked bad block marker, this will
    U32 BadBlockMarkerStartBit; //!< For BCH ECC sizes other than 8 and 16 the bad block marker does not start 
    U32 bbm_physical_off; 		//!< FCB value that gives byte offset for bad block marker on physical NAND page.
    U32 bch_type;               //!< 0 for BCH20 and 1 for BCH32
    TMNANDFCBData togglemodeNANDData;   //!< Info needed for configuring clocks and timing parameters for togglemode nand.
    U32 disbbm;                 // 1--disable bbm, 0 enable bbm
    U32 bbm_spare_offset;
    U32 Onfi_sync_enable;
    U32 Onfi_sync_speed;        //0-20m;1-33m;2-50m;3-66m;4-83m;5-100m;
    OnfisyncFCBData Onfi_syncNANDData;
    U32 DISBB_Search;
	U32 rand_en;       			// enable randomizer, randomizer type 0x2
    U32 rsvd[FCB_RSVD_NUM];     // reserved
    U32 read_retry_en;
    U32 read_retry_seq_len;
    U8 read_retry_seq[MAX_READ_RETRY_SEQ_LEN];
} FCBStruct_t;

/*=================================================================================
                                 Global Functions
=================================================================================*/
BOOL nand_boot_chip_cfg(chip_e chip);
BOOL nand_boot_fcb_search(U32 search_stride, U8 * nand_buffer, FCBStruct_t ** pFCB);
BOOL nand_boot_init_param(void);
BOOL nand_boot_set_param(FCBStruct_t * pFCB);
BOOL nand_boot_init(void);

#endif /*ROM_NAND_BOOT_H */
