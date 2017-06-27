#ifndef _QSPI_PRIVATE_H_
#define _QSPI_PRIVATE_H_

#include "io.h"
#include "soc_info.h"
#include "platform.h"

/* QSPI - Peripheral register structure */
typedef struct QSPI_MemMap {
  UINT32 MCR;                   /* offset: 0x00000000*/
  UINT32 TCR;                   /* offset: 0x00000004 */
  UINT32 IPCR;                  /* offset: 0x00000008*/
  UINT32 FLSHCR;                /* offset: 0x0000000C*/
  UINT32 BUF0CR;                /* offset: 0x00000010*/
  UINT32 BUF1CR;                /* offset: 0x00000014*/
  UINT32 BUF2CR;                /* offset: 0x00000018*/
  UINT32 BUF3CR;                /* offset: 0x0000001C*/
  UINT32 BFGENCR;               /* offset: 0x00000020*/
  UINT32 SOCCR;                 /* offset: 0x00000024*/
  UINT8 RESERVED1[8];           /* offset: 0x00000028*/
  UINT32 BUF0IND;               /* offset: 0x00000030*/
  UINT32 BUF1IND;               /* offset: 0x00000034*/
  UINT32 BUF2IND;               /* offset: 0x00000038*/
  UINT8 RESERVED2[196];
  UINT32 SFAR;                  /* offset: 0x00000100*/
  UINT8 RESERVED3[4];
  UINT32 SMPR;                  /* offset: 0x00000108*/
  UINT32 RBSR;                  /* offset: 0x0000010C*/
  UINT32 RBCT;                  /* offset: 0x00000110*/
  UINT8 RESERVED4[60];
  UINT32 TBSR;                  /* offset: 0x00000150*/
  UINT32 TBDR;                  /* offset: 0x00000154*/
  UINT8 RESERVED5[4];
  UINT32 SR;                    /* offset: 0x0000015C*/
  UINT32 FR;                    /* offset: 0x00000160*/
  UINT32 RSER;                  /* offset: 0x00000164*/
  UINT32 SPNDST;                /* offset: 0x00000168*/
  UINT32 SPTRCLR;               /* offset: 0x0000016C*/
  UINT8 RESERVED6[16];
  UINT32 SFA1AD;                /* offset: 0x00000180*/
  UINT32 SFA2AD;                /* offset: 0x00000184*/
  UINT32 SFB1AD;                /* offset: 0x00000188*/
  UINT32 SFB2AD;                /* offset: 0x0000018C*/
  UINT8 RESERVED7[112];
  UINT32 RBDR[32];              /* offset: 0x00000200--0x0000027C*/
  UINT8 RESERVED8[128];
  UINT32 LUTKEY;                /* offset: 0x00000300*/
  UINT32 LCKCR;                 /* offset: 0x00000304*/
  UINT8 RESERVED9[8];
  UINT32 LUT[64];               /* offset: 0x00000310 -- 0x0000040C */
} volatile *QSPI_MemMapPtr;

extern void qspi_init(QSPI_MemMapPtr instance);
extern uint32_t qspi_program(QSPI_MemMapPtr instance, uint32_t off, uint32_t src, uint32_t size);
extern uint32_t qspi_read_id(QSPI_MemMapPtr instance);
extern void qspi_chip_erase(QSPI_MemMapPtr instance);
extern void qspi_sector_erase(QSPI_MemMapPtr instance, uint32_t addr);
extern void qspi_write_non_volatile(QSPI_MemMapPtr instance, uint32_t status);
extern uint32_t qspi_read_status(QSPI_MemMapPtr instance);
extern uint32_t qspi_read_config(QSPI_MemMapPtr instance);

// ---------------------------------------------------------------
// Memory Map
// ---------------------------------------------------------------
#define QSPI1_FLASH_A1_BASE  0x60000000
#define QSPI1_FLASH_A1_TOP   0x64000000
#define QSPI1_FLASH_A2_BASE  0x64000000
#define QSPI1_FLASH_A2_TOP   0x68000000
#define QSPI1_FLASH_B1_BASE  0x68000000
#define QSPI1_FLASH_B1_TOP   0x6C000000
#define QSPI1_FLASH_B2_BASE  0x6C000000
#define QSPI1_FLASH_B2_TOP   0x70000000

#define QSPI2_FLASH_A1_BASE  0x70000000
#define QSPI2_FLASH_A1_TOP   0x74000000
#define QSPI2_FLASH_A2_BASE  0x74000000
#define QSPI2_FLASH_A2_TOP   0x78000000
#define QSPI2_FLASH_B1_BASE  0x78000000
#define QSPI2_FLASH_B1_TOP   0x7C000000
#define QSPI2_FLASH_B2_BASE  0x7C000000
#define QSPI2_FLASH_B2_TOP   0x80000000

// ---------------------------------------------------------------
// Enumeration & Structure
// ---------------------------------------------------------------
enum QSPI_INST_E {
  INST_STOP = 0x0,
  INST_CMD = 0x1,
  INST_ADDR = 0x2,
  INST_DUMMY = 0x3,
  INST_MODE = 0x4,
  INST_MODE2 = 0x5,
  INST_MODE4 = 0x6,
  INST_READ = 0x7,
  INST_WRITE = 0x8,
  INST_JMP_ON_CS = 0x9,
  INST_ADDR_DDR = 0xA,
  INST_MODE_DDR = 0xB,
  INST_MODE2_DDR = 0xC,
  INST_MODE4_DDR = 0xD,
  INST_READ_DDR = 0xE,
  INST_WRITE_DDR = 0xF,
  INST_DATA_LEARN = 0x10
};

enum QSPI_PAD_E {
  PAD_1X = 0x0,
  PAD_2X = 0x1,
  PAD_4X = 0x2,
  PAD_RSVD = 0x3
};

// Command Set for Macronix Serial Flash
enum QSPI_CMD_E {
  WREN        = 0x06, // WriteEnable
  WRDI        = 0x04, // WriteDisable
  RDID        = 0x9F, // ReadID
  RDSR        = 0x05, // ReadStatus
  WRSR        = 0x01, // WriteStatus
  RD1X        = 0x03, // ReadData
  RD2X        = 0xbb, // 2X Read
  RD4X        = 0xeb, // 4XI/O Read;
  FASTRD1X    = 0x0b, // FastReadData
  DTRD1X      = 0x0d, // fast DDR read 1XI/O Read;
  DTRD2X      = 0xbd, // Dual DDR read 2XI/O Read;
  DTRD4X      = 0xed, // Quad DDR read 4XI/O Read;
  SE          = 0x20, // SectorErase
  BE2         = 0x52, // 32k block erase
  BE          = 0xd8, // BlockErase
  CE1         = 0x60, // ChipErase
  CE2         = 0xc7, // ChipErase
  PP          = 0x02, // PageProgram
  DP          = 0xb9, // DeepPowerDown
  RDPRES      = 0xab, // ReleaseFromDeepPowerDwon
  REMS        = 0x90, // ReadElectricManufacturerDeviceID
  REMS2       = 0xef, // ReadElectricManufacturerDeviceID
  REMS4       = 0xdf, // ReadElectricManufacturerDeviceID
  REMS4D      = 0xcf, // ReadElectricManufacturerDeviceID
  CP          = 0xad, // Continuously  program mode;
  ENSO        = 0xb1, // Enter secured OTP;
  EXSO        = 0xc1, // Exit  secured OTP;
  RDSCUR      = 0x2b, // Read  security  register;
  WRSCUR      = 0x2f, // Write security  register;
  ESRY        = 0x70, // Enable  SO to output RY/BY;
  DSRY        = 0x80, // Disable SO to output RY/BY;
  RDSFDP      = 0x5a, // enter SFDP read mode
  HPM         = 0xa3, // High Performance Enable Mode
  PP4X        = 0x38, // 4I Page Pgm load address and data all 4io
  CLSR        = 0x30, // clear the security register E_FAIL bit and P_FAIL bit
  ENPLM       = 0x55, // enter parallel mode
  EXPLM       = 0x45, // exit parallel mode
  WPSEL       = 0x68, // write protection selection
  SBLK        = 0x36, // single block lock
  SBULK       = 0x39, // single block unlock
  RDBLOCK     = 0x3c, // block protect read
  GBLK        = 0x7e, // gang block lock
  GBULK       = 0x98  // gang block unlock
};

#endif



