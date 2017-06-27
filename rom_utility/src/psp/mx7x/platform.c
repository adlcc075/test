#include "platform.h"
#include "io.h"

/*
 * Fortunately, all the three mx6x platform share the same USB phy, CCM memory map.
 */

extern unsigned int USDHC_mx7d_iomux_cfg[];

extern unsigned int NAND_mx7d_iomux_cfg[];

uint32_t regs_OCOTP_base;

SOC_MEM_MAP_ENTRY(OCOTP)

uint32_t regs_IOMUXC_base;

SOC_MEM_MAP_ENTRY(IOMUXC)

uint32_t regs_USDHC1_base;

SOC_MEM_MAP_ENTRY(USDHC1)

uint32_t regs_USDHC2_base;

SOC_MEM_MAP_ENTRY(USDHC2)

uint32_t regs_USDHC3_base;

SOC_MEM_MAP_ENTRY(USDHC3)

uint32_t regs_USDHC4_base;

SOC_MEM_MAP_ENTRY(USDHC4)

uint32_t regs_WEIM_CS_base;

SOC_MEM_MAP_ENTRY(WEIM_CS)

/* apbh, gpmi, bch */
uint32_t iomux_NAND_cfg;
IOMUX_CFG_TBL_DEF(NAND)

uint32_t regs_APBH_DMA_base;
SOC_MEM_MAP_ENTRY(APBH_DMA)

uint32_t regs_GPMI_base;
SOC_MEM_MAP_ENTRY(GPMI)

uint32_t regs_BCH_base;
SOC_MEM_MAP_ENTRY(BCH)

uint32_t regs_USB_base;
SOC_MEM_MAP_ENTRY(USB)

