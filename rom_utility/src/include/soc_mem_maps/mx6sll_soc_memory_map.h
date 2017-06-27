
#ifndef _MX6SLL_SOC_MEMORY_MAP_H
#define _MX6SLL_SOC_MEMORY_MAP_H

//#define BAAD_STATUS                     0xbaadbaad
//#define GOOD_STATUS                     0x900d900d

// Cortex-A9 MPCore private memory region
#define ARM_PERIPHBASE                  0x00A00000
#define SCU_BASE_ADDR_MX6SLL                   ARM_PERIPHBASE
#define IC_INTERFACES_BASE_ADDR_MX6SLL         (ARM_PERIPHBASE+0x0100)
#define GLOBAL_TIMER_BASE_ADDR_MX6SLL          (ARM_PERIPHBASE+0x0200)
#define PRIVATE_TIMERS_WD_BASE_ADDR_MX6SLL     (ARM_PERIPHBASE+0x0600)
#define IC_DISTRIBUTOR_BASE_ADDR_MX6SLL        (ARM_PERIPHBASE+0x1000)

// CPU Memory Map
#define MMDC0_ARB_BASE_ADDR_MX6SLL      0x80000000
#define MMDC0_ARB_END_ADDR_MX6SLL       0xBFFFFFFF
#define MMDC1_ARB_BASE_ADDR_MX6SLL      0xC0000000
#define MMDC1_ARB_END_ADDR_MX6SLL       0xFFFFFFFF

#define OCRAM_ARB_BASE_ADDR_MX6SLL       0x00900000
#define OCRAM_ARB_END_ADDR_MX6SLL       0x009FFFFF

#define IRAM_BASE_ADDR_MX6SLL            OCRAM_ARB_BASE_ADDR_MX6SLL

// Legacy Defines
#define CSD0_DDR_BASE_ADDR_MX6SLL              MMDC0_ARB_BASE_ADDR_MX6SLL
#define CSD1_DDR_BASE_ADDR_MX6SLL              0xC0000000

// s_g_N ports
#define AIPS1_ARB_BASE_ADDR_MX6SLL       0x02000000
#define AIPS1_ARB_END_ADDR_MX6SLL        0x020FFFFF
#define AIPS2_ARB_BASE_ADDR_MX6SLL       0x02100000
#define AIPS2_ARB_END_ADDR_MX6SLL        0x021FFFFF

#define ROMCP_ARB_BASE_ADDR_MX6SLL             0x00000000
#define ROMCP_ARB_END_ADDR_MX6SLL              0x00017FFF
#define BOOT_ROM_BASE_ADDR_MX6SLL              ROMCP_ARB_BASE_ADDR_MX6SLL



// CoreSight (ARM Debug)
#define DEBUG_ROM_BASE_ADDR_MX6SLL             0x02140000
#define ETB_BASE_ADDR_MX6SLL                   0x02141000
#define EXT_CTI_BASE_ADDR_MX6SLL               0x02142000
#define TPIU_BASE_ADDR_MX6SLL                  0x02143000
#define FUNNEL_BASE_ADDR_MX6SLL                0x02144000
#define CORTEX_ROM_TABLE                0x0214F000
#define CORTEX_DEBUG_UNIT               0x02150000
#define CORE0_DEBUG_UNIT                0x02150000
#define PMU0_BASE_ADDR_MX6SLL                  0x02151000
#define CORE1_DEBUG_UNIT                0x02152000
#define PMU1_BASE_ADDR_MX6SLL                  0x02153000
#define CORE2_DEBUG_UNIT                0x02154000
#define PMU2_BASE_ADDR_MX6SLL                  0x02155000
#define CORE3_DEBUG_UNIT                0x02156000
#define PMU3_BASE_ADDR_MX6SLL                  0x02157000
#define CTI0_BASE_ADDR_MX6SLL                  0x02158000
#define CTI1_BASE_ADDR_MX6SLL                  0x02159000
#define CTI2_BASE_ADDR_MX6SLL                  0x0215A000
#define CTI3_BASE_ADDR_MX6SLL                  0x0215B000
#define PTM0_BASE_ADDR_MX6SLL                  0x0215C000
#define PTM_BASE_ADDR_MX6SLL                  0x0215C000
#define PTM1_BASE_ADDR_MX6SLL                  0x0215D000
#define PTM2_BASE_ADDR_MX6SLL                  0x0215E000
#define PTM3_BASE_ADDR_MX6SLL                  0x0215F000


// Defines for Blocks connected via AIPS (SkyBlue)
#define AIPS_TZ1_BASE_ADDR_MX6SLL              AIPS1_ARB_BASE_ADDR_MX6SLL
#define AIPS_TZ2_BASE_ADDR_MX6SLL              AIPS2_ARB_BASE_ADDR_MX6SLL

#define SPDIF_BASE_ADDR_MX6SLL             (AIPS_TZ1_BASE_ADDR_MX6SLL + 0x04000) 
#define ECSPI1_BASE_ADDR_MX6SLL            (AIPS_TZ1_BASE_ADDR_MX6SLL + 0x08000) 
#define ECSPI2_BASE_ADDR_MX6SLL            (AIPS_TZ1_BASE_ADDR_MX6SLL + 0x0C000) 
#define ECSPI3_BASE_ADDR_MX6SLL            (AIPS_TZ1_BASE_ADDR_MX6SLL + 0x10000) 
#define ECSPI4_BASE_ADDR_MX6SLL            (AIPS_TZ1_BASE_ADDR_MX6SLL + 0x14000)
#define UART4_BASE_ADDR_MX6SLL             (AIPS_TZ1_BASE_ADDR_MX6SLL + 0x18000)
#define UART1_BASE_ADDR_MX6SLL             (AIPS_TZ1_BASE_ADDR_MX6SLL + 0x20000)
#define UART2_BASE_ADDR_MX6SLL             (AIPS_TZ1_BASE_ADDR_MX6SLL + 0x24000)
#define SSI1_BASE_ADDR_MX6SLL              (AIPS_TZ1_BASE_ADDR_MX6SLL + 0x28000)
#define SSI2_BASE_ADDR_MX6SLL              (AIPS_TZ1_BASE_ADDR_MX6SLL + 0x2c000)
#define SSI3_BASE_ADDR_MX6SLL              (AIPS_TZ1_BASE_ADDR_MX6SLL + 0x30000)
#define UART3_BASE_ADDR_MX6SLL             (AIPS_TZ1_BASE_ADDR_MX6SLL + 0x34000) 
#define SPBA_BASE_ADDR_MX6SLL              (AIPS_TZ1_BASE_ADDR_MX6SLL + 0x3c000)

// AIPS_TZ#1- On Platform
#define AIPS1_ON_BASE_ADDR_MX6SLL          (AIPS_TZ1_BASE_ADDR_MX6SLL + 0x0007C000)    // 0x0207C000
// AIPS_TZ#1- Off Platform
#define AIPS1_OFF_BASE_ADDR_MX6SLL         (AIPS_TZ1_BASE_ADDR_MX6SLL + 0x00080000)    // 0x02080000

#define PWM1_BASE_ADDR_MX6SLL              (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x0000)
#define PWM2_BASE_ADDR_MX6SLL              (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x4000)
#define PWM3_BASE_ADDR_MX6SLL              (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x8000)
#define PWM4_BASE_ADDR_MX6SLL              (AIPS1_OFF_BASE_ADDR_MX6SLL + 0xC000)
#define QOSC_BASE_ADDR_MX6SLL              (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x00014000)   // 0x02094000
#define GPT_BASE_ADDR_MX6SLL               (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x00018000)   // 0x02098000
#define GPIO1_BASE_ADDR_MX6SLL             (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x0001C000)   // 0x0209C000
#define GPIO2_BASE_ADDR_MX6SLL             (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x00020000)   // 0x020A0000
#define GPIO3_BASE_ADDR_MX6SLL             (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x00024000)   // 0x020A4000
#define GPIO4_BASE_ADDR_MX6SLL             (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x00028000)   // 0x020A8000
#define GPIO5_BASE_ADDR_MX6SLL             (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x0002C000)   // 0x020AC000
#define GPIO6_BASE_ADDR_MX6SLL             (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x00030000)   // 0x020B0000
#define KPP_BASE_ADDR_MX6SLL               (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x00038000)   // 0x020B8000
#define WDOG1_BASE_ADDR_MX6SLL             (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x0003C000)   // 0x020BC000
#define WDOG2_BASE_ADDR_MX6SLL             (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x00040000)   // 0x020C0000
#define CCM_BASE_ADDR_MX6SLL               (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x00044000)   // 0x020C4000
#define ANATOP_BASE_ADDR_MX6SLL            (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x00048000)   // 0x020C8000		same as CCM_ANALOG	above
#define SNVS_BASE_ADDR_MX6SLL              (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x0004C000)   // 0x020CC000
#define EPIT1_BASE_ADDR_MX6SLL             (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x00050000)   // 0x020D0000
#define EPIT2_BASE_ADDR_MX6SLL             (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x00054000)   // 0x020D4000
#define SRC_BASE_ADDR_MX6SLL               (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x00058000)   // 0x020D8000
#define GPC_BASE_ADDR_MX6SLL               (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x0005C000)   // 0x020DC000		same as DVFS below
#define IOMUXC_BASE_ADDR_MX6SLL            (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x00060000)   // 0x020E0000
#define IOMUXC_GPR_BASE_ADDR_MX6SLL        (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x00064000)   // 0x020E4000
#define CSI_BASE_ADDR_MX6SLL               (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x00068000)   // 0x020E8000
#define SDMA_BASE_ADDR_MX6SLL              (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x0006c000)   // 0x020EC000
#define EPXP_BASE_ADDR_MX6SLL              (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x00070000)   // 0x020F0000
#define EPDC_BASE_ADDR_MX6SLL              (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x00074000)   // 0x020F4000
#define LCDIF_BASE_ADDR_MX6SLL             (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x00078000)   // 0x020F8000
#define DCP_BASE_ADDR_MX6SLL               (AIPS1_OFF_BASE_ADDR_MX6SLL + 0x0007c000)   // 0x020FC000



// (AIPS_TZ#2- On Platform
#define AIPS2_ON_BASE_ADDR_MX6SLL              (AIPS_TZ2_BASE_ADDR_MX6SLL+0x7C000)
// (AIPS_TZ#2- Off Platform
#define AIPS2_OFF_BASE_ADDR_MX6SLL             (AIPS_TZ2_BASE_ADDR_MX6SLL+0x80000)


#define USBO2H_PL301_BASE_ADDR_MX6SLL         (AIPS2_OFF_BASE_ADDR_MX6SLL+0x00000)
#define USB_BASE_ADDR_MX6SLL            	  (AIPS2_OFF_BASE_ADDR_MX6SLL+0x04000)
#define USDHC1_BASE_ADDR_MX6SX                (AIPS2_OFF_BASE_ADDR_MX6SLL+0x10000)
#define USDHC2_BASE_ADDR_MX6SX                (AIPS2_OFF_BASE_ADDR_MX6SLL+0x14000)
#define USDHC3_BASE_ADDR_MX6SX                (AIPS2_OFF_BASE_ADDR_MX6SLL+0x18000)
#define I2C1_BASE_ADDR_MX6SX                  (AIPS2_OFF_BASE_ADDR_MX6SLL+0x20000)
#define I2C2_BASE_ADDR_MX6SX                  (AIPS2_OFF_BASE_ADDR_MX6SLL+0x24000)
#define I2C3_BASE_ADDR_MX6SX                  (AIPS2_OFF_BASE_ADDR_MX6SLL+0x28000)
#define ROMCP_BASE_ADDR_MX6SX                 (AIPS2_OFF_BASE_ADDR_MX6SLL+0x2C000)
#define MMDC_P0_BASE_ADDR_MX6SX               (AIPS2_OFF_BASE_ADDR_MX6SLL+0x30000)
#define RNGB_BASE_ADDR_MX6SLL                 (AIPS2_OFF_BASE_ADDR_MX6SLL+0x34000)
#define OCOTP_BASE_ADDR_MX6SLL                (AIPS2_OFF_BASE_ADDR_MX6SLL+0x3C000)
#define CSU_BASE_ADDR_MX6SX                   (AIPS2_OFF_BASE_ADDR_MX6SLL+0x40000)
#define IOMUXC_BASE_ADDR_MX6SLL               (AIPS2_OFF_BASE_ADDR_MX6SLL+0x44000)
#define IOMUXC_GPR_BASE_ADDR_MX6SLL           (AIPS2_OFF_BASE_ADDR_MX6SLL+0x48000)
#define TZASC1_BASE_ADDR_MX6SLL               (AIPS2_OFF_BASE_ADDR_MX6SLL+0x50000)
#define AUDMUX_BASE_ADDR_MX6SLL               (AIPS2_OFF_BASE_ADDR_MX6SLL+0x58000)
#define UART5_BASE_ADDR_MX6SLL                (AIPS2_OFF_BASE_ADDR_MX6SLL+0x74000)
#define MTR_MASTER_BASE_ADDR_MX6SLL           (AIPS2_OFF_BASE_ADDR_MX6SLL+0x7C000)




#endif