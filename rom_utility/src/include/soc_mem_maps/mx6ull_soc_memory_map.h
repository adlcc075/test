
#ifndef _MX6ULL_SOC_MEMORY_MAP_H
#define _MX6ULL_SOC_MEMORY_MAP_H

#define BAAD_STATUS                     0xbaadbaad
#define GOOD_STATUS                     0x900d900d



#define GIC400_ARB_BASE_ADDR            0xA00000

// CPU Memory Map
// Blocks connected via pl301fast
// s_a_N, s_d_N ports
#define MMDC0_ARB_BASE_ADDR             0x80000000
#define MMDC0_ARB_END_ADDR              0xBFFFFFFF
#define MMDC1_ARB_BASE_ADDR             0xC0000000
#define MMDC1_ARB_END_ADDR              0xFFFFFFFF
#define OCRAM_ARB_BASE_ADDR             0x00900000
#define OCRAM_ARB_END_ADDR              0x009FFFFF
#define IRAM_BASE_ADDR                  OCRAM_ARB_BASE_ADDR
#define QSPI0_ARB_BASE_ADDR             0x60000000 
#define QSPI0_ARB_END_ADDR              0x6FFFFFFF 
#define QSPI1_ARB_BASE_ADDR             0x70000000 
#define QSPI1_ARB_END_ADDR              0x7FFFFFFF
#define QS0_BUFFER_BASE_ADDR            0x0C000000
#define QS0_BUFFER_END_ADDR             0x0DFFFFFF
#define QS1_BUFFER_BASE_ADDR            0x0E000000
#define QS1_BUFFER_END_ADDR             0x0FFFFFFF
//no pcie
//#define PCIE_ARB_BASE_ADDR              0x08000000
//#define PCIE_ARB_END_ADDR               0x08FFFFFF

// Blocks connected via pl301periph
// s_e_N ports
#define ROMCP_ARB_BASE_ADDR             0x00000000
#define ROMCP_ARB_END_ADDR              0x00017FFF
#define BOOT_ROM_BASE_ADDR              ROMCP_ARB_BASE_ADDR
#define CAAM_ARB_BASE_ADDR              0x00100000
#define CAAM_ARB_END_ADDR               0x00107FFF
//no gpu2d
//#define GPU_2D_ARB_BASE_ADDR            0x01800000
//#define GPU_2D_ARB_END_ADDR             0x01803FFF
#define APBH_DMA_ARB_END_ADDR           0x0180BFFF
// apbh, gpmi, bch
#define APBH_DMA_BASE_ADDR_MX6ULL		0x01804000
#define GPMI_BASE_ADDR_MX6ULL			0x01806000
#define BCH_BASE_ADDR_MX6ULL				0x01808000

//#define OPENVG_ARB_BASE_ADDR           0x02204000
//#define OPENVG_ARB_END_ADDR            0x02207FFF
// GPV - PL301 configuration ports
#define GPV0_BASE_ADDR                  0x00B00000
#define GPV1_BASE_ADDR                  0x00C00000
#define GPV2_BASE_ADDR                  0x00D00000
#define GPV3_BASE_ADDR                  0x00E00000
//#define GPV4_BASE_ADDR                  0x00F00000
//#define GPV5_BASE_ADDR                  0x01000000
//#define GPV6_BASE_ADDR                  0x01100000
// s_g_N ports
#define AIPS1_ARB_BASE_ADDR             0x02000000
#define AIPS1_ARB_END_ADDR              0x020FFFFF
#define AIPS2_ARB_BASE_ADDR             0x02100000
#define AIPS2_ARB_END_ADDR              0x021FFFFF
//#define AIPS3_ARB_BASE_ADDR             0x02200000
//#define AIPS3_ARB_END_ADDR              0x022FFFFF


#define WEIM_ARB_BASE_ADDR_MX6ULL              0x50000000
#define WEIM_ARB_END_ADDR               0x5FFFFFFF

#define WEIM_CS_BASE_ADDR_MX6ULL		WEIM_ARB_BASE_ADDR_MX6ULL


#if 0
// CoreSight (ARM Debug)
// ***** TO UPDATE *****
#define DEBUG_ROM_BASE_ADDR             0x02100000
#define ETB_BASE_ADDR                   0x02101000
#define ETF_BASE_ADDR                   0x02101000
#define EXT_CTI_BASE_ADDR               0x02102000
#define TPIU_BASE_ADDR                  0x02103000
#define TSGEN_BASE_ADDR                 0x02104000
#define CORTEX_ROM_TABLE                0x02120000
#define CORTEX_DEBUG_UNIT               0x02130000
#define CORE0_DEBUG_UNIT                0x02130000
#define PMU0_BASE_ADDR                  0x02131000
#define CORE1_DEBUG_UNIT                0x02132000
#define PMU1_BASE_ADDR                  0x02133000
#define CORE2_DEBUG_UNIT                0x02134000
#define PMU2_BASE_ADDR                  0x02135000
#define CORE3_DEBUG_UNIT                0x02136000
#define PMU3_BASE_ADDR                  0x02137000
#define CTI0_BASE_ADDR                  0x02138000
#define CTI1_BASE_ADDR                  0x02139000
#define CTI2_BASE_ADDR                  0x0213A000
#define CTI3_BASE_ADDR                  0x0213B000
#define ETM_BASE_ADDR                   0x0213C000
#define PTM0_BASE_ADDR                  0x0213C000
#define PTM1_BASE_ADDR                  0x0213D000
#define PTM2_BASE_ADDR                  0x0213E000
#define PTM3_BASE_ADDR                  0x0213F000
// *********************
#endif

// Legacy Defines
#define CSD0_DDR_BASE_ADDR              MMDC0_ARB_BASE_ADDR
#define CSD1_DDR_BASE_ADDR              0xC0000000
#define CS0_BASE_ADDR                   WEIM_ARB_BASE_ADDR
// Defines for Blocks connected via AIPS (SkyBlue)
#define AIPS_TZ1_BASE_ADDR              AIPS1_ARB_BASE_ADDR
#define AIPS_TZ2_BASE_ADDR              AIPS2_ARB_BASE_ADDR
//#define AIPS_TZ3_BASE_ADDR              AIPS3_ARB_BASE_ADDR

//slots 0,7,14 of SDMA reserved, therefore left unused in IPMUX3
#define SPDIF_IPS_BASE_ADDR             AIPS_TZ1_BASE_ADDR+0x04000 //slot 1
#define ECSPI1_IPS_BASE_ADDR            AIPS_TZ1_BASE_ADDR+0x08000 //slot 2
#define ECSPI2_IPS_BASE_ADDR            AIPS_TZ1_BASE_ADDR+0x0C000 //slot 3
#define ECSPI3_IPS_BASE_ADDR            AIPS_TZ1_BASE_ADDR+0x10000 //slot 4
#define ECSPI4_IPS_BASE_ADDR            AIPS_TZ1_BASE_ADDR+0x14000 //slot 5
#define UART7_IPS_BASE_ADDR             AIPS_TZ1_BASE_ADDR+0x18000 //slot 6 added
#define UART1_IPS_BASE_ADDR             AIPS_TZ1_BASE_ADDR+0x20000 //slot 8
#define UART8_IPS_BASE_ADDR             AIPS_TZ1_BASE_ADDR+0x24000 //slot 9 fix
#define SAI1_IPS_BASE_ADDR              AIPS_TZ1_BASE_ADDR+0x28000 //slot 10
#define SAI2_IPS_BASE_ADDR              AIPS_TZ1_BASE_ADDR+0x2C000 //slot 11
#define SAI3_IPS_BASE_ADDR              AIPS_TZ1_BASE_ADDR+0x30000 //slot 12 
#define ASRC_IPS_BASE_ADDR              AIPS_TZ1_BASE_ADDR+0x34000 //slot 13
#define SPBA_IPS_BASE_ADDR              AIPS_TZ1_BASE_ADDR+0x3C000 //slot 15
#define TSC_DIG_IPS_BASE_ADDR           AIPS_TZ1_BASE_ADDR+0x40000 

// AIPS_TZ#1- On Platform
#define AIPS1_ON_BASE_ADDR              AIPS_TZ1_BASE_ADDR+0x7C000

// AIPS_TZ#1- Off Platform
//ex- #define AIPS1_BASE_ADDR                 AIPS_TZ1_BASE_ADDR+0x80000
#define AIPS1_OFF_BASE_ADDR             AIPS_TZ1_BASE_ADDR+0x80000
//#define USBOH3_BASE_ADDR              AIPS1_BASE_ADDR

#define PWM1_IPS_BASE_ADDR              AIPS1_OFF_BASE_ADDR+0x0000
#define PWM2_IPS_BASE_ADDR              AIPS1_OFF_BASE_ADDR+0x4000
#define PWM3_IPS_BASE_ADDR              AIPS1_OFF_BASE_ADDR+0x8000
#define PWM4_IPS_BASE_ADDR              AIPS1_OFF_BASE_ADDR+0xC000
#define CAN1_IPS_BASE_ADDR              AIPS1_OFF_BASE_ADDR+0x10000
#define CAN2_IPS_BASE_ADDR              AIPS1_OFF_BASE_ADDR+0x14000
#define GPT1_IPS_BASE_ADDR              AIPS1_OFF_BASE_ADDR+0x18000 //fix
#define GPT_IPS_BASE_ADDR               GPT1_IPS_BASE_ADDR           //set GPT1 default
#define GPIO1_IPS_BASE_ADDR             AIPS1_OFF_BASE_ADDR+0x1C000
#define GPIO2_IPS_BASE_ADDR             AIPS1_OFF_BASE_ADDR+0x20000
#define GPIO3_IPS_BASE_ADDR             AIPS1_OFF_BASE_ADDR+0x24000
#define GPIO4_IPS_BASE_ADDR             AIPS1_OFF_BASE_ADDR+0x28000
#define GPIO5_IPS_BASE_ADDR             AIPS1_OFF_BASE_ADDR+0x2C000
#define SNVS_LP_IPS_BASE_ADDR             AIPS1_OFF_BASE_ADDR+0x30000
#define ENET2_IPS_BASE_ADDR             AIPS1_OFF_BASE_ADDR+0x34000
#define KPP_IPS_BASE_ADDR               AIPS1_OFF_BASE_ADDR+0x38000
#define WDOG1_IPS_BASE_ADDR             AIPS1_OFF_BASE_ADDR+0x3C000
#define WDOG2_IPS_BASE_ADDR             AIPS1_OFF_BASE_ADDR+0x40000
#define CCM_IPS_BASE_ADDR_MX6ULL               AIPS1_OFF_BASE_ADDR+0x44000
#define ANATOP_IPS_BASE_ADDR_MX6ULL            AIPS1_OFF_BASE_ADDR+0x48000
#define SNVS_IPS_BASE_ADDR              AIPS1_OFF_BASE_ADDR+0x4C000
#define EPIT1_IPS_BASE_ADDR_MX6ULL             AIPS1_OFF_BASE_ADDR+0x50000
#define EPIT2_IPS_BASE_ADDR             AIPS1_OFF_BASE_ADDR+0x54000
#define SRC_IPS_BASE_ADDR               AIPS1_OFF_BASE_ADDR+0x58000
#define GPC_IPS_BASE_ADDR               AIPS1_OFF_BASE_ADDR+0x5C000
#define IOMUXC_IPS_BASE_ADDR            AIPS1_OFF_BASE_ADDR+0x60000
#define IOMUXC_BASE_ADDR_MX6ULL                IOMUXC_IPS_BASE_ADDR 
#define IOMUXC_GPR_BASE_ADDR            AIPS1_OFF_BASE_ADDR+0x64000
#define GPT2_IPS_BASE_ADDR              AIPS1_OFF_BASE_ADDR+0x68000 //fix
#define SDMA_PORT_IPS_HOST_BASE_ADDR    AIPS1_OFF_BASE_ADDR+0x6C000
#define PWM5_IPS_BASE_ADDR              AIPS1_OFF_BASE_ADDR+0x70000 //fix
#define PWM6_IPS_BASE_ADDR              AIPS1_OFF_BASE_ADDR+0x74000 //fix
#define PWM7_IPS_BASE_ADDR              AIPS1_OFF_BASE_ADDR+0x78000 //fix
#define PWM8_IPS_BASE_ADDR              AIPS1_OFF_BASE_ADDR+0x7C000 //fix


// AIPS_TZ#2- On Platform
#define AIPS2_ON_BASE_ADDR              AIPS_TZ2_BASE_ADDR+0x7C000

// AIPS_TZ#2- Off Platform
#define AIPS2_OFF_BASE_ADDR             AIPS_TZ2_BASE_ADDR+0x80000

// AIPS_TZ#2  - Global enable (0)
#define CAAM_IPS_BASE_ADDR              AIPS_TZ2_BASE_ADDR+0x40000
#define ARM_IPS_BASE_ADDR	        AIPS_TZ2_BASE_ADDR

#define USBO2H_PL301_IPS_BASE_ADDR          AIPS2_OFF_BASE_ADDR+0x0000
#define USB_BASE_ADDR_MX6ULL            		  (AIPS2_OFF_BASE_ADDR_MX6SX+0x4000)
#define ENET1_IPS_BASE_ADDR                  AIPS2_OFF_BASE_ADDR+0x8000
#define SIM1_IPS_BASE_ADDR                  AIPS2_OFF_BASE_ADDR+0xC000 //new add
#define USDHC1_BASE_ADDR_MX6ULL                AIPS2_OFF_BASE_ADDR+0x10000
#define USDHC2_BASE_ADDR_MX6ULL                AIPS2_OFF_BASE_ADDR+0x14000
#define USDHC3_BASE_ADDR_MX6ULL                0
#define USDHC4_BASE_ADDR_MX6ULL                0
#define ADC1_IPS_BASE_ADDR                  AIPS2_OFF_BASE_ADDR+0x18000 //fix
#define ADC2_IPS_BASE_ADDR                  AIPS2_OFF_BASE_ADDR+0x1C000 //fix
#define I2C1_IPS_BASE_ADDR                  AIPS2_OFF_BASE_ADDR+0x20000
#define I2C2_IPS_BASE_ADDR                  AIPS2_OFF_BASE_ADDR+0x24000
#define I2C3_IPS_BASE_ADDR                  AIPS2_OFF_BASE_ADDR+0x28000
#define ROMCP_BASE_ADDR_MX6ULL                 AIPS2_OFF_BASE_ADDR+0x2C000 
#define MMDC_P0_IPS_BASE_ADDR               AIPS2_OFF_BASE_ADDR+0x30000
#define SIM2_IPS_BASE_ADDR                  AIPS2_OFF_BASE_ADDR+0x34000 //fix
#define WEIM_BASE_ADDR_MX6ULL                  AIPS2_OFF_BASE_ADDR+0x38000
#define OCOTP_BASE_ADDR_MX6ULL                 AIPS2_OFF_BASE_ADDR+0x3C000
#define CSU_IPS_BASE_ADDR                   AIPS2_OFF_BASE_ADDR+0x40000
#define CSI_IPS_BASE_ADDR      		    AIPS2_OFF_BASE_ADDR+0x44000 //fix
#define ELCDIF_IPS_BASE_ADDR 	            AIPS2_OFF_BASE_ADDR+0x48000 //fix
#define EPXP_IPS_BASE_ADDR        	    AIPS2_OFF_BASE_ADDR+0x4C000 //fix
#define IP2APB_TZASC1_IPS_BASE_ADDR         AIPS2_OFF_BASE_ADDR+0x50000
#define SYSCNT_RD_IPS_BASE_ADDR             AIPS2_OFF_BASE_ADDR+0x54000 //fix
#define SYSCNT_CMP_BASE_ADDR                AIPS2_OFF_BASE_ADDR+0x58000 //fix
#define SYSCNT_CTRL_IPS_BASE_ADDR           AIPS2_OFF_BASE_ADDR+0x5C000 //fix
#define QSPI_BASE_ADDR_MX6ULL                  AIPS2_OFF_BASE_ADDR+0x60000 //fix
#define WDOG3_IPS_BASE_ADDR                 AIPS2_OFF_BASE_ADDR+0x64000 //fix
#define UART2_IPS_BASE_ADDR                 AIPS2_OFF_BASE_ADDR+0x68000   
#define UART3_IPS_BASE_ADDR                 AIPS2_OFF_BASE_ADDR+0x6C000
#define UART4_IPS_BASE_ADDR                 AIPS2_OFF_BASE_ADDR+0x70000
#define UART5_IPS_BASE_ADDR                 AIPS2_OFF_BASE_ADDR+0x74000
#define I2C4_IPS_BASE_ADDR                  AIPS2_OFF_BASE_ADDR+0x78000
#define UART6_IPS_BASE_ADDR                 AIPS2_OFF_BASE_ADDR+0x7C000 //fix
//#define IP2APB_USBPHY1_IPS_BASE_ADDR        AIPS2_OFF_BASE_ADDR+0x78000
//#define IP2APB_USBPHY2_IPS_BASE_ADDR        AIPS2_OFF_BASE_ADDR+0x7C000

//// AIPS_TZ#3- On Platform
//#define AIPS3_ON_BASE_ADDR              AIPS_TZ3_BASE_ADDR+0x7C000
//
//// AIPS_TZ#3- Off Platform
//#define AIPS3_OFF_BASE_ADDR             AIPS_TZ3_BASE_ADDR+0x80000
//
////on-plat peripherals
//#define GIS_IPS_BASE_ADDR               AIPS_TZ3_BASE_ADDR+0x4000
//#define DCIC1_IPS_BASE_ADDR             AIPS_TZ3_BASE_ADDR+0xC000
//#define DCIC2_IPS_BASE_ADDR             AIPS_TZ3_BASE_ADDR+0x10000
//#define CSI1_IPS_BASE_ADDR              AIPS_TZ3_BASE_ADDR+0x14000
//#define EPXP_IPS_BASE_ADDR              AIPS_TZ3_BASE_ADDR+0x18000
//#define CSI2_IPS_BASE_ADDR              AIPS_TZ3_BASE_ADDR+0x1C000
//#define ELCDIF1_IPS_BASE_ADDR           AIPS_TZ3_BASE_ADDR+0x20000
//#define ELCDIF2_IPS_BASE_ADDR           AIPS_TZ3_BASE_ADDR+0x24000
//#define VADC_IPS_BASE_ADDR              AIPS_TZ3_BASE_ADDR+0x28000
//#define VDEC_IPS_BASE_ADDR              AIPS_TZ3_BASE_ADDR+0x2C000
//#define TZ3_SPBA_IPS_BASE_ADDR          AIPS_TZ3_BASE_ADDR+0x3C000
//
//#define ADC1_IPS_BASE_ADDR              AIPS3_OFF_BASE_ADDR
//#define ADC2_IPS_BASE_ADDR              AIPS3_OFF_BASE_ADDR+0x4000
//#define WDOG3_IPS_BASE_ADDR             AIPS3_OFF_BASE_ADDR+0x8000
//#define ECSPI5_IPS_BASE_ADDR            AIPS3_OFF_BASE_ADDR+0xC000
//#define HS_IPS_BASE_ADDR                AIPS3_OFF_BASE_ADDR+0x10000
//#define MUCPU_IPS_BASE_ADDR             AIPS3_OFF_BASE_ADDR+0x14000
//#define CANFDCPU_IPS_BASE_ADDR          AIPS3_OFF_BASE_ADDR+0x18000
//#define MUDSP_IPS_BASE_ADDR             AIPS3_OFF_BASE_ADDR+0x1C000
//#define UART6_IPS_BASE_ADDR             AIPS3_OFF_BASE_ADDR+0x20000
//#define PWM5_IPS_BASE_ADDR              AIPS3_OFF_BASE_ADDR+0x24000
//#define PWM6_IPS_BASE_ADDR              AIPS3_OFF_BASE_ADDR+0x28000
//#define PWM7_IPS_BASE_ADDR              AIPS3_OFF_BASE_ADDR+0x2C000
//#define PWM8_IPS_BASE_ADDR              AIPS3_OFF_BASE_ADDR+0x30000
//


//#define AHBMAX_IPS_BASE_ADDR			    0x63F94000
//#define MAX_IPS_BASE_ADDR			        AHBMAX_BASE_ADDR 
//#define IIM_IPS_BASE_ADDR			        0x63F98000
//#define FIRI_IPS_BASE_ADDR			        0x63FA8000
#define SDMA_IPS_HOST_BASE_ADDR	        SDMA_PORT_IPS_HOST_BASE_ADDR
#define SDMA_IPS_HOST_IPS_BASE_ADDR     SDMA_PORT_IPS_HOST_BASE_ADDR
//#define SCC_IPS_BASE_ADDR			        0x63FB4000
//#define RTICV3_IPS_BASE_ADDR			    0x63FBC000
//#define RTIC_IPS_BASE_ADDR			        RTICV3_BASE_ADDR 
//#define CSPI_IPS_BASE_ADDR			        0x63FC0000
//#define RTC_IPS_BASE_ADDR			        0x63FD4000
//#define M4IF_REGISTERS_IPS_BASE_ADDR        0x63FD8000
//#define ESDCTL_REGISTERS_IPS_BASE_ADDR      0x63FD9000
//#define WEIM_REGISTERS_IPS_BASE_ADDR        0x63FDA000 
//#define EIM_IPS_BASE_ADDR                   WEIM_REGISTERS_BASE_ADDR 
//#define EMIV2_REGISTERS_IPS_BASE_ADDR       0x63FDBF00
//#define PL301_2X2_IPS_BASE_ADDR			    0x63FDC000
//#define PL301_4X1_IPS_BASE_ADDR			    0x63FE0000
//#define FEC_IPS_BASE_ADDR			        0x63FEC000
//#define TVE_IPS_BASE_ADDR			        0x63FF0000
//#define SAHARA_IPS_BASE_ADDR		    	0x63FF8000
//#define PTP_IPS_BASE_ADDR    		    	0x63FFC000

// Cortex-A7 MPCore private memory region
#define ARM_PERIPHBASE                  0x00A00000
#define SCU_BASE_ADDR                   ARM_PERIPHBASE
#define IC_INTERFACES_BASE_ADDR         ARM_PERIPHBASE+0x0100
#define GLOBAL_TIMER_BASE_ADDR          ARM_PERIPHBASE+0x0200
#define PRIVATE_TIMERS_WD_BASE_ADDR     ARM_PERIPHBASE+0x0600
#define IC_DISTRIBUTOR_BASE_ADDR        ARM_PERIPHBASE+0x1000

#ifdef PORTHOLE_ADDR_IN_RAM
  #define PORTHOLE_ADDR_IN_RAM_1
#endif

#ifdef TEST_PORT_LOAD_MODE
  #define PORTHOLE_ADDR_IN_RAM_1
#endif

/* Porthole address (for verilog trigger) */

   #ifdef PORTHOLE_ADDR_IN_RAM_1
     // OCRAM real is 0x900000..0x93FFFF, 2 CS, we choose to sit in end of first by default,
  #ifdef USE_HIGH_RAM_4SPECIAL_ADDR
    #define PORTHOLE_ADDR                 IRAM_BASE_ADDR + 0x1E000
  #else //not USE_HIGH_RAM_4SPECIAL_ADDR
    #define PORTHOLE_ADDR                 IRAM_BASE_ADDR + 0x1F000 //0x10FF0
  #endif // ifdef USE_HIGH_RAM_4SPECIAL_ADDR
#else //PORTHOLE_ADDR_IN_RAM
  #ifdef PORTHOLE_ADDR_IN_CS1
    #define PORTHOLE_ADDR             (CS0_BASE_ADDR + 0x04000000 + 0xA1C)
  #else // ifdef PORTHOLE_ADDR_IN_CS1
    #define PORTHOLE_ADDR             (CS0_BASE_ADDR + 0x7feff00)         // DEFAULT
  #endif // ifdef PORTHOLE_ADDR_IN_CS1
#endif // ifdef PORTHOLE_ADDR_IN_RAM

#ifdef TEST_PORT_LOAD_MODE //ARM_EMBEDDED_MODE
// Tester Message defines
#define CHECKSUM_START       0x11223344
#define CHECKSUM_END         0x44332211
#define CHECKSUM_PASSED      0x11112222
#define CHECKSUM_FAILED      0x44443333
#define INIT_VALUE           0x5a5a5a5a
#define PRELOAD_START        0x11111111
#define PRELOAD_END          0x22222222
#define DOWNLOAD_START       0x33333333
#define DOWNLOAD_END         0x44444444
#define TEST_RUN_START       0x55555555
#define TEST_RUN_END         0x66666666
#define CLN_RESULTS_START    0x77777777
#define CLN_RESULTS_END      0x88888888
#define UPLOAD_START         0x99999999
#define UPLOAD_END           0xaaaaaaaa
#define MAIN_START           0xbbbbbbbb
#define MAIN_END             0xcccccccc
#define TEST_PASSED          0x900d900d
#define TEST_FAILED          0xbaadbaad
#define DEBUG01              0xdddd0001
#define DEBUG02              0xdddd0002
#define DEBUG03              0xdddd0003
#define DEBUG04              0xdddd0004
#define DEBUG05              0xdddd0005
#define DEBUG06              0xdddd0006
#define DEBUG07              0xdddd0007
#define DEBUG08              0xdddd0008
#define DEBUG09              0xdddd0009
#define PAD_CRL_WRITE_START  0xeeeeeeee
#define PAD_CRL_WRITE_END    0xffffffff
// Production mem map
// External Mem (CS0) Memory Map
#define CS0_PRELOAD_ADDR               CS0_BASE_ADDR
#define CS0_EIM2RAM_ADDR               CS0_BASE_ADDR+0x2000
#define CS0_RESULTS_ADDR               CS0_BASE_ADDR+0x8000
  #define CS0_TEST_CODE_SIZE_ADDR         CS0_BASE_ADDR+0xfffc
  #define CS0_TEST_CODE_ADDR              CS0_BASE_ADDR+0x10000
//#define TESTER_MSG_ADDR                   CS0_BASE_ADDR+0x100000

                                         

  //Updated for Arik
  //Here the test itself starts
  #define RAM_TEST_CODE_ADDR             IRAM_BASE_ADDR  // 0x90_0000..0x90_4800 apprx. the test + all start/end routines
  //Here a routine that used to load the test from Weim to Cache was
  //copied in the beginning, and then run from there. Not used in Test Port Mode
  #define RAM_EIM2RAM_ADDR               IRAM_BASE_ADDR+0x5000     // // 90_5000 - 90_57FF (7FF)   eimtomem routine

#ifdef USE_HIGH_RAM_4SPECIAL_ADDR

  #define TESTER_MSG_ADDR                IRAM_BASE_ADDR+0x1E080
  #define RAM_RESULT_STAT_ADDR           IRAM_BASE_ADDR+0x1E090   // 4 byte, verilog_trigger, for saving good/baad status
  #define RAM_RESULT_COUN_ADDR           IRAM_BASE_ADDR+0x1E094   // 4 byte, counter for saved events in
  //RAM_MEM_RESULT_BASE_ADDR + event_counter*4 < RAM_MEM_RESULT_END by verilog trigger
  #define RAM_RESULT_CMP_COUN_ADDR       IRAM_BASE_ADDR+0x1E098   // 4 byte, counter for saved events in
  //RAM_MEM_CMP_RESULT_BASE_ADDR + event_counter*4 < RAM_MEM_CMP_RESULT_END by info trigger
  #define RAM_MULTI_CORE_TALK_ADDR       IRAM_BASE_ADDR+0x1E0A0   // 4 byte, address for 4 cores to signal their state
  #define RAM_REAL_CHECKSUM_ADDR         IRAM_BASE_ADDR+0x1E0B0   // 4 byte, place to store checksun real result - should be 0
  #define RAM_MULTI_CORE_STATUS0_0       IRAM_BASE_ADDR+0x1E0C0   // 4 byte, address for core0 first result in multi core mode
  #define RAM_MULTI_CORE_STATUS0_1       IRAM_BASE_ADDR+0x1E0C4   // 4 byte, address for core1 first result in multi core mode
  #define RAM_MULTI_CORE_STATUS0_2       IRAM_BASE_ADDR+0x1E0C8   // 4 byte, address for core2 first result in multi core mode
  #define RAM_MULTI_CORE_STATUS0_3       IRAM_BASE_ADDR+0x1E0CC   // 4 byte, address for core3 first result in multi core mode
  #define RAM_MULTI_CORE_STATUS1_0       IRAM_BASE_ADDR+0x1E0D0   // 4 byte, address for core0 second result in multi core mode
  #define RAM_MULTI_CORE_STATUS1_1       IRAM_BASE_ADDR+0x1E0D4   // 4 byte, address for core1 second result in multi core mode
  #define RAM_MULTI_CORE_STATUS1_2       IRAM_BASE_ADDR+0x1E0D8   // 4 byte, address for core2 second result in multi core mode
  #define RAM_MULTI_CORE_STATUS1_3       IRAM_BASE_ADDR+0x1E0DC   // 4 byte, address for core3 second result in multi core mode
  #define RAM_INTERRUPT_0                IRAM_BASE_ADDR+0x1E0E0   // 4 byte, stores value copied from ICDABR0 register
  #define RAM_INTERRUPT_1                IRAM_BASE_ADDR+0x1E0E4   // 4 byte, stores value copied from ICDABR1 register
  #define RAM_INTERRUPT_2                IRAM_BASE_ADDR+0x1E0E8   // 4 byte, stores value copied from ICDABR2 register
  #define RAM_INTERRUPT_3                IRAM_BASE_ADDR+0x1E0EC   // 4 byte, stores value copied from ICDABR3 register
  #define RAM_MEM_RESULT_BASE_ADDR       IRAM_BASE_ADDR+0x1E100   // 93_E100 - 
  #define RAM_MEM_RESULT_END             IRAM_BASE_ADDR+0x1E7FF   // - 93_E7FF (0x6FF)   Result memory, 1.75 KB
  #define RAM_MEM_CMP_RESULT_BASE_ADDR   IRAM_BASE_ADDR+0x1E800   // 93_E400 - 
  #define RAM_MEM_CMP_RESULT_END         IRAM_BASE_ADDR+0x1EFFF   // - 93_E7FF (0x800)   Compare Result memory, 2 KB
  #define CHECKSUM_STORE_ADDR            IRAM_BASE_ADDR+0x1F000   //  1st is amount of other words, each record is 3 words: start address, stop address, checksum
  //This is for general usage of other tests, lets put in CS1
  #define RAM_FREE_START                 IRAM_BASE_ADDR+0x10000 // 92_0000
  #define RAM_FREE_END                   IRAM_BASE_ADDR+0x1E000 // 93_FFB0          - F801_FFB0 (FFB0)  Free Space
#else  //!USE_HIGH_RAM_4SPECIAL_ADDR
  //Here the 1F... addresses are put for consistency, but since they are used only in TEST_PORT_LOAD_MODE,
  //the definition below are not suppoosed to be used
  #define TESTER_MSG_ADDR                IRAM_BASE_ADDR+0x1F080
  #define RAM_RESULT_STAT_ADDR           IRAM_BASE_ADDR+0x1F090   // 4 byte, verilog_trigger, for saving good/baad status
  #define RAM_RESULT_COUN_ADDR           IRAM_BASE_ADDR+0x1F094   // 4 byte, counter for saved events in
  //RAM_MEM_RESULT_BASE_ADDR + event_counter*4 < RAM_MEM_RESULT_END by verilog trigger
  #define RAM_RESULT_CMP_COUN_ADDR       IRAM_BASE_ADDR+0x1F098   // 4 byte, counter for saved events in
  //RAM_MEM_CMP_RESULT_BASE_ADDR + event_counter*4 < RAM_MEM_CMP_RESULT_END by info trigger
  #define RAM_MULTI_CORE_TALK_ADDR       IRAM_BASE_ADDR+0x1F0A0   // 4 byte, address for 4 cores to signal their state
  #define RAM_REAL_CHECKSUM_ADDR         IRAM_BASE_ADDR+0x1F0B0   // 4 byte, place to store checksun real result - should be 0
  #define RAM_MULTI_CORE_STATUS0_0       IRAM_BASE_ADDR+0x1F0C0   // 4 byte, address for core0 first result in multi core mode
  #define RAM_MULTI_CORE_STATUS0_1       IRAM_BASE_ADDR+0x1F0C4   // 4 byte, address for core1 first result in multi core mode
  #define RAM_MULTI_CORE_STATUS0_2       IRAM_BASE_ADDR+0x1F0C8   // 4 byte, address for core2 first result in multi core mode
  #define RAM_MULTI_CORE_STATUS0_3       IRAM_BASE_ADDR+0x1F0CC   // 4 byte, address for core3 first result in multi core mode
  #define RAM_MULTI_CORE_STATUS1_0       IRAM_BASE_ADDR+0x1F0D0   // 4 byte, address for core0 second result in multi core mode
  #define RAM_MULTI_CORE_STATUS1_1       IRAM_BASE_ADDR+0x1F0D4   // 4 byte, address for core1 second result in multi core mode
  #define RAM_MULTI_CORE_STATUS1_2       IRAM_BASE_ADDR+0x1F0D8   // 4 byte, address for core2 second result in multi core mode
  #define RAM_MULTI_CORE_STATUS1_3       IRAM_BASE_ADDR+0x1F0DC   // 4 byte, address for core3 second result in multi core mode
  #define RAM_INTERRUPT_0                IRAM_BASE_ADDR+0x1F0E0   // 4 byte, stores value copied from ICDABR0 register
  #define RAM_INTERRUPT_1                IRAM_BASE_ADDR+0x1F0E4   // 4 byte, stores value copied from ICDABR1 register
  #define RAM_INTERRUPT_2                IRAM_BASE_ADDR+0x1F0E8   // 4 byte, stores value copied from ICDABR2 register
  #define RAM_INTERRUPT_3                IRAM_BASE_ADDR+0x1F0EC   // 4 byte, stores value copied from ICDABR3 register
  #define RAM_MEM_RESULT_BASE_ADDR       IRAM_BASE_ADDR+0x1F100   // 91_F100 - 
  #define RAM_MEM_RESULT_END             IRAM_BASE_ADDR+0x1F3FF   // - 91_F3FF (0x2EF)   Result memory, 0.75 KB
  #define RAM_MEM_CMP_RESULT_BASE_ADDR   IRAM_BASE_ADDR+0x1F400   // 91_F400 - 
  #define RAM_MEM_CMP_RESULT_END         IRAM_BASE_ADDR+0x1F7FF   // - 91_F7FF (0x400)   Compare Result memory, 1 KB
  #define CHECKSUM_STORE_ADDR            IRAM_BASE_ADDR+0x1E000   //  1st is amount of other words, each record is 3 words: start address, stop address, checksum
  //This is for general usage of other tests, lets put in CS1
  #define RAM_FREE_START                 IRAM_BASE_ADDR+0x20000 // 92_0000
  #define RAM_CHECKSUM_INIT_FUNC         IRAM_BASE_ADDR+0x1F000   // - small function defining start addr, size and checksum for each block to check
  #define RAM_FREE_END                   IRAM_BASE_ADDR+0x1EFFC
#endif  //!USE_HIGH_RAM_4SPECIAL_ADDR

  //End of RAM, used by interrupt fuctions, Now in 0x93FFB8
  #define RAM_SPECIAL_FUNCTION           IRAM_BASE_ADDR+0x1FFB8 // 93_FFB8 - 93_FFFF (48)    Special function IRQ FIQ ABORT UNDEF


#endif  //TEST_PORT_LOAD_MODE

#ifdef PROD_MODE_GMEM
// Tester Message defines
#define INIT_VALUE           0x5a5a5a5a
#define PRELOAD_START        0x11111111
#define PRELOAD_END          0x22222222
#define DOWNLOAD_START       0x33333333
#define DOWNLOAD_END         0x44444444
#define TEST_RUN_START       0x55555555
#define TEST_RUN_END         0x66666666
#define CLN_RESULTS_START    0x77777777
#define CLN_RESULTS_END      0x88888888
#define UPLOAD_START         0x99999999
#define UPLOAD_END           0xaaaaaaaa
#define MAIN_START           0xbbbbbbbb
#define MAIN_END             0xcccccccc
#define TEST_PASSED          0x900d900d
#define TEST_FAILED          0xbaadbaad
#define DEBUG01              0xdddd0001
#define DEBUG02              0xdddd0002
#define DEBUG03              0xdddd0003
#define DEBUG04              0xdddd0004
#define DEBUG05              0xdddd0005
#define DEBUG06              0xdddd0006
#define DEBUG07              0xdddd0007
#define DEBUG08              0xdddd0008
#define DEBUG09              0xdddd0009
#define PAD_CRL_WRITE_START  0xeeeeeeee
#define PAD_CRL_WRITE_END    0xffffffff
// Production mem map
// External Mem (CS0) Memory Map
#define CS0_PRELOAD_ADDR               CS0_BASE_ADDR
#define CS0_EIM2RAM_ADDR               CS0_BASE_ADDR+0x2000
#define CS0_RESULTS_ADDR               CS0_BASE_ADDR+0x8000
#ifdef ARM_EMBEDDED_MODE_CACHE_HEX
  #define CS0_TEST_CODE_SIZE_ADDR         CS0_BASE_ADDR+0x1fffc
  #define CS0_TEST_CODE_ADDR              CS0_BASE_ADDR+0x20000
#else // ARM_EMBEDDED_MODE_CACHE_HEX
  #define CS0_TEST_CODE_SIZE_ADDR         CS0_BASE_ADDR+0xfffc
  #define CS0_TEST_CODE_ADDR              CS0_BASE_ADDR+0x10000
#endif // ARM_EMBEDDED_MODE_CACHE_HEX
#define TESTER_MSG_ADDR                   CS0_BASE_ADDR+0x100000
                                         
#ifdef ARM_EMBEDDED_MODE_CACHE
  #define RAM_FREE_START                 IRAM_BASE_ADDR+0x00000 // F800_0000
  #define RAM_FREE_END                   IRAM_BASE_ADDR+0x13FFF //           - F801_3FFF (14000) Free Space
  #define CACHE_TRANLATION_TABLE         IRAM_BASE_ADDR+0x14000 // F801_4000 - F801_7FFF (4000)  Cache Page table
  #define RAM_EIM2RAM_ADDR               IRAM_BASE_ADDR+0x18000 // F801_8000 - F801_87FF (7FF)   eim2cache routine
  #define STACK_START                    IRAM_BASE_ADDR+0x18800 // F801_8800
  #define STACK_END                      IRAM_BASE_ADDR+0x18FEF //           - F801_8FEF (7EF)   Stack
  #define RAM_RESULT_STAT_ADDR           IRAM_BASE_ADDR+0x1E000 // F801_E000 -                   Test status
  #define RAM_RESULT_COUN_ADDR           IRAM_BASE_ADDR+0x1E004 // F801_E004 -                   Results counter
  #define RAM_MEM_RESULT_BASE_ADDR       IRAM_BASE_ADDR+0x1E008 // F801_E008 -
  #define RAM_MEM_RESULT_END             IRAM_BASE_ADDR+0x1EFFF //           - F801_EFFF (FF7)   Results area
  #define RAM_SPECIAL_FUNCTION           IRAM_BASE_ADDR+0x1FFBC // F801_FFBC - F801_FEFF (43)    Special function IRQ FIQ ABORT UNDEF
#else // ARM_EMBEDDED_MODE_CACHE
  #define RAM_EIM2RAM_ADDR               IRAM_BASE_ADDR         // F800_0000 - F800_07FF (7FF)   eimtomem routine
  #define RAM_RESULT_STAT_ADDR           IRAM_BASE_ADDR+0x800   // F800_0800 -
  #define RAM_RESULT_COUN_ADDR           IRAM_BASE_ADDR+0x804   // F800_0804 -
  #define RAM_MEM_RESULT_BASE_ADDR       IRAM_BASE_ADDR+0x808   // F800_0808 - 
  #define RAM_MEM_RESULT_END             IRAM_BASE_ADDR+0xFEF   //           - F800_0FEF (7EF)   Result memory
  #define RAM_TEST_CODE_ADDR             IRAM_BASE_ADDR+0x1000  // F800_1000 - F801_7FFF (AFFF)  Test code
  #define RAM_FREE_START                 IRAM_BASE_ADDR+0x10000 // F801_0000
  #define RAM_FREE_END                   IRAM_BASE_ADDR+0x1FFB0 //           - F801_FFB0 (FFB0)  Free Space
  #define RAM_SPECIAL_FUNCTION           IRAM_BASE_ADDR+0x1FFBC // F801_FFBC - F801_FEFF (43)    Special function IRQ FIQ ABORT UNDEF
#endif  // ARM_EMBEDDED_MODE_CACHE

#endif  //PROD_MODE_GMEM


#endif


