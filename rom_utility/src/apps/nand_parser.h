#ifndef __NAND_PARSER_H__
#define __NAND_PARSER_H__

#include "io.h"
#include "regs.h"

#define MAX_COLUMNS     2       //!< Current NANDs only use 2 bytes for column.
#define MAX_ROWS        5       //!< Current NANDs use a max of 3 bytes for row.
//! CLE command plus up to Max Columns and Max Rows.
#define CLE1_MAX_SIZE   MAX_COLUMNS+MAX_ROWS+1

typedef struct _dma_cmd
{
    struct _dma_cmd    *pNxt;
    UINT32   cmd;
    void                *pBuf;
    UINT32     ctrl;
    UINT32   cmp;
} dma_cmd_t;

typedef struct _apbh_dma_t
{
    struct _apbh_dma_t*   nxt;
    UINT32     cmd;
    void*                 bar;
} apbh_dma_t;

typedef struct _apbh_dma_gpmi1_t
{
    struct _apbh_dma_gpmi1_t*	nxt;
    UINT32           cmd;
    void*                       bar;
    union
    {
        struct
        {
            UINT32       gpmi_ctrl0;
        }apbh_dma_gpmi1_ctrl;
        reg32_t                   pio[1];
    }apbh_dma_gpmi1_u;
} apbh_dma_gpmi1_t;

typedef struct _apbh_dma_gpmi2_t
{
    struct _apbh_dma_gpmi2_t*     nxt;
    UINT32             cmd;
    void*                         bar;
    union
    {
        struct
        {
            UINT32       gpmi_ctrl0;
            UINT32     gpmi_compare;
        }apbh_dma_gpmi2_ctrlcompare;
        reg32_t                   pio[2];
    }apbh_dma_gpmi2_u;
} apbh_dma_gpmi2_t;

typedef struct _apbh_dma_gpmi3_t
{
    struct _apbh_dma_gpmi1_t*	  nxt;
    UINT32             cmd;
    void*                         bar;
    union
    {
        struct
        {
            UINT32       gpmi_ctrl0;
            UINT32     gpmi_compare;
            UINT32     gpmi_eccctrl;
        }apbh_dma_gpmi3_ctrl;
        reg32_t                   pio[3];
    }apbh_dma_gpmi3_u;
} apbh_dma_gpmi3_t;

typedef struct _apbh_dma_gpmi4_t
{
    struct _apbh_dma_gpmi1_t*	  nxt;
    UINT32             cmd;
    void*                         bar;
    union
    {
        struct
        {
            UINT32       gpmi_ctrl0;
            UINT32     gpmi_compare;
            UINT32     gpmi_eccctrl;
            UINT32    gpmi_ecccount;
        }apbh_dma_gpmi4_reg;
        reg32_t                   pio[4];
    }apbh_dma_gpmi4_u;
} apbh_dma_gpmi4_t;

typedef struct _apbh_dma_gpmi5_t
{
    struct _apbh_dma_gpmi1_t*	  nxt;
    UINT32             cmd;
    void*                         bar;
    union
    {
        struct
        {
            UINT32       gpmi_ctrl0;
            UINT32     gpmi_compare;
            UINT32     gpmi_eccctrl;
            UINT32    gpmi_ecccount;
            UINT32     gpmi_payload;
        }apbh_dma_gpmi5_reg;
        reg32_t                   pio[5];
    }apbh_dma_gpmi5_u;
} apbh_dma_gpmi5_t;

typedef struct _apbh_dma_gpmi6_t
{
    struct _apbh_dma_gpmi1_t*	  nxt;
    UINT32             cmd;
    void*                         bar;
    union
    {
        struct
        {
            UINT32       gpmi_ctrl0;
            UINT32     gpmi_compare;
            UINT32     gpmi_eccctrl;
            UINT32    gpmi_ecccount;
            UINT32     gpmi_payload;
            UINT32   gpmi_auxiliary;
        }apbh_dma_gpmi6_ctrl;
        reg32_t                   pio[6];
    }apbh_dma_gpmi6_u;
} apbh_dma_gpmi6_t;

typedef struct _NAND_read_seed_t
{
    // Number of Column & Row bytes to be sent.
    UINT32    uiAddressSize;
    // How many bytes of data do we want to read back?
    UINT32    uiReadSize;
    // How many chunks of 512 bytes should be ECCed?
    UINT32    uiECCMask;
    // Enable or Disable ECC
    BOOL        bEnableHWECC;

    union
    {
        // 1 byte CLE, up to 5 bytes of Column & Row.
        reg8_t  tx_cle1_addr_buf[MAX_COLUMNS+MAX_ROWS+1];
        struct
        {
            reg8_t tx_cle1;
            union
            {
                reg8_t tx_addr[MAX_COLUMNS+MAX_ROWS];
                // Type2 array has 2 Columns & 3 Rows.
                struct
                {
                    reg8_t  bType2Columns[MAX_COLUMNS];
                    reg8_t  bType2Rows[MAX_ROWS];
                }tx_cle1_Type2;
                // Type1 array has 1 Column & up to 3 Rows
                struct
                {
                    reg8_t  bType1Columns[1];
                    reg8_t  bType1Rows[MAX_ROWS];
                }tx_cle1_Type1;
            }tx_cle1_Columns_Rows;            
        }tx_cle1_addr_Columns_Rows;
    }tx_cle1_addr_dma_buffer;

    union
    {
        reg8_t  tx_cle2_buf[1];
        struct
        {
            reg8_t tx_cle2;
        }tx_cle2_addr_dma;
    }tx_cle2_addr_dma_buffer;

    // Buffer pointer for data
    void * pDataBuffer;
    // Buffer pointer for Auxiliary data (Redundant and ECC)..
    void * pAuxBuffer;

} NAND_read_seed_t;

//! \brief DMA Descriptor structure for a NAND Read transaction.
//!
//! This structure defines the DMA chains required to send a Device 
//! Read sequence to the NAND.  The following chains are required:
//!     Transmit Read Command and Address - Send primary command and address to the NAND.
//!     Transmit Read2 Command - Send second read command to the NAND.
//!     Wait for Ready - waits for the device to enter the Ready state.
//!     Sense Ready - Sense if the device is ready, if not branch to error routine.
//!     Receive Data - Read the data page from the NAND.
//!     Disable ECC - Disable the ECC DMA (flow-through DMA on Encore).
typedef struct _NAND_dma_read_t
{
    // descriptor sequence
    apbh_dma_gpmi3_t  tx_cle1_addr_dma;
    apbh_dma_gpmi1_t  tx_cle2_dma;
    apbh_dma_gpmi1_t  wait_dma;
    apbh_dma_gpmi1_t  sense_dma;
    apbh_dma_gpmi1_t  wait_dma1;
    apbh_dma_gpmi1_t  sense_dma1;
    apbh_dma_gpmi6_t  rx_data_dma;
    apbh_dma_gpmi3_t  rx_wait4done_dma;

    // terminator functions
    apbh_dma_t        success_dma;
    apbh_dma_t        timeout_dma;

    // Add the DMA Read Seed into the Read DMA structure.
    NAND_read_seed_t  NAND_DMA_Read_Seed;

} NAND_dma_read_t;

typedef struct _NandDmaTime_t
{
    UINT32    uStartDMATime;
    UINT32   uDMATimeout;
} NandDmaTime_t;

typedef struct _NANDSectorDescriptorStruct_t {
  UINT32    u32total_page_sz;	    // Total Page size - if >2K page use actual size.  
  UINT16    u16SectorSize; // Sector Size field from BA NAND's parameters page.
} NANDSectorDescriptorStruct_t;

///////////////////////////////////////////////////////////////////////////////
// NAND Command Codes Descriptor
typedef struct _NANDCommandCodesStruct_t {

  // NOTE: Command are a single byte, in the LSByte.  A value of -1 (0xff) indicates
  //       a code is not available for the current device.  This may cause problems
  //       because Reset typeically uses 0xFF.

  UINT8 btReadIDCode;                    // ReadID
  UINT8 btResetCode;                     // Reset

  UINT8 btRead1Code;                     // Read (Mode 1)
  UINT8 btRead1_2ndCycleCode;            // Second Cycle for Read (Type 2 NANDs)

} NANDCommandCodesStruct_t;

typedef struct _NAND_Descriptor{
  NANDSectorDescriptorStruct_t stc_NANDSectorDescriptor;
  NANDCommandCodesStruct_t stc_NANDDeviceCommandCodes;  
  UINT8	btNumRowBytes;		// Number of Row Address bytes required
  UINT8	btNumColBytes;		// Number of Row Address bytes required
  UINT8	btNANDDataBusWidth;	// BusWidth = 8 or 16 bits
						// Use this parameter only to initialize the global CurrentNANDBusWidth
						// Some NANDs required more real time process to determine  their bus
						// width. (see CurrentNANDBusWidth declaration Note for further information)
  UINT32  uiNANDTotalBlocks;
  UINT32  u32NumLBAPerRead; // Number of addressed LBAs per 2K bytes
  bool      bBA_NAND;
} NAND_Descriptor_t;

typedef struct _NandConfiguration
{
//-----------------information from fcb--------------------
    UINT32 data_page_sz;       /*data pagesize*/
    UINT32 total_page_sz;

    UINT32 blockn_ecc_level;          /*< Type of ECC, can be one of BCH-0-32*/
    UINT32 block0_sz;             /*< Number of bytes for Block0 - BCH*/
    UINT32 blockn_sz;             //!< Block size in bytes for all blocks other than Block0 - BCH
    UINT32 block0_ecc_level;          //!< Ecc level for Block 0 - BCH
    UINT32 meta_sz;             //!< Metadata size - BCH
    UINT32 bch_type;                  //!< 0 for BCH20 and 1 for BCH32
    UINT32 blockn_num_per_page;
    NandDmaTime_t  zNandDmaTime; 
    UINT32 bbm_byte_off_in_data; //!< FCB value that gives byte offset for bad block marker in data area
    UINT32 bbm_bit_off; //!< FCB value that gives starting bit offset within m_u32bbm_byte_off_in_data
    UINT32 bbm_physical_off; 
    UINT32 bbm_spare_offset;//BBM data position stored at metadata
	
    UINT32 OnfiSync_Enable;
    UINT32 OnfiSync_Speed;
    UINT32 DISBB_Search;
//---------------- data struncture used for DMA-----------------
    bool bEnableHWECC;             /* 8 or 16 bit data width */
    NAND_dma_read_t DmaReadDma;     //!< Read DMA descriptor
    NAND_Descriptor_t zNANDDescriptor;
// ---------------information read from efuse------------------	
    UINT32 efNANDReadCmdCode1;
    UINT32 efNANDReadCmdCode2;
    UINT32 pages_per_block;       /* 32, 64 or 128 pages per block */
    UINT32 AddressCycles;       /* Select address cycles number between 3-6 */           
    UINT32 boot_search_limit;
    
} nand_info_t;

typedef struct _NAND_Timing
{        
    UINT8 m_u8DataSetup;
    UINT8 m_u8DataHold;
    UINT8 m_u8AddressSetup;
    UINT8 m_u8Reserved;
    // These are for application use only and not for ROM.
    UINT8 m_u8NandTimingState; 
    UINT8 m_u8REA;
    UINT8 m_u8RLOH;
    UINT8 m_u8RHOH;
} NAND_Timing_t;

typedef struct _NandBootData 
{
    nand_info_t  NandConf;
    UINT32               disbbm;
    UINT32               dbbtNotFound;
    UINT32               Firmware1_startingSector;  /* Page address of primare firmware */
    UINT32               Firmware2_startingSector;  /* Page address of secondary firmware */
    UINT32               BadBlocksNum;              /* Total number of bad blocks found in DBBT */
    UINT32*              BadBlocksTable;            /* Bad blocks table */

} NandBootData_t;

int nand_parser(NandBootData_t *nand);

#endif
