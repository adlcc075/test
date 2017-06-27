

#ifndef _BOOT_CFG_H_
#define _BOOT_CFG_H_

typedef enum {
    WEIMNOR_BOOT,
    QSPINOR_BOOT,
    SPINOR_BOOT,
    SD_BOOT,
    MMC_BOOT,
    NAND_BOOT,
} BOOT_DEV_e;

extern u32 boot_cfg_override(BOOT_DEV_e dev, u32 port, chip_e chip, u8 *para);

#endif

