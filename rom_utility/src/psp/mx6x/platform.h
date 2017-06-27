#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "soc_info.h"
#include "soc_mem_maps/mx6dq_soc_memory_map.h"
#include "soc_mem_maps/mx6sdl_soc_memory_map.h"
#include "soc_mem_maps/mx6sl_soc_memory_map.h"
#include "soc_mem_maps/mx6sx_soc_memory_map.h"
#include "soc_mem_maps/mx6ul_soc_memory_map.h"
#include "soc_mem_maps/mx6ull_soc_memory_map.h"

typedef enum {
    USDHC_PORT1 = 0,
    USDHC_PORT2 = 1,
    USDHC_PORT3 = 2,
    USDHC_PORT4 = 3,
    USDHC_NUMBER_PORTS = 4
} usdhc_port_e;

typedef enum {
    SPI_PORT1 = 0,
    SPI_PORT2 = 1,
    SPI_PORT3 = 2,
    SPI_PORT4 = 3,
    SPI_NUMBER_PORTS = 4
} spi_port_e;

typedef enum {
    I2C_PORT1 = 0,
    I2C_PORT2 = 1,
    I2C_PORT3 = 2,
    I2C_NUMBER_PORTS = 3,
} i2c_port_e;

typedef enum {
    QUAD_SPI_1 = 0,
    QUAD_SPI_2 = 1,
    QUAD_SPI_INVAL = 0xffffffff,
} qspi_port_e;


#define ANATOP_IPS_BASE_ADDR      	0x020C8000
#define HW_CCM_ANALOG_USBPHY0_PLL_480_CTRL_ADDR		(ANATOP_IPS_BASE_ADDR + 0x10)
#define HW_CCM_ANALOG_USBPHY1_PLL_480_CTRL_ADDR		(ANATOP_IPS_BASE_ADDR + 0x20)

#define IP2APB_USBPHY1_BASE_ADDR  0x020C9000
#define IP2APB_USBPHY2_BASE_ADDR  0x020CA000

#define CCM_IPS_BASE_ADDR	0x020C4000
#define CCM_CCGR6_OFFSET 	0x80
#define CCM_CCGR6	(CCM_IPS_BASE_ADDR + CCM_CCGR6_OFFSET)
#define CCM_CSCDR1_OFFSET 0x24
#define CCM_CSCDR1  CCM_IPS_BASE_ADDR+CCM_CSCDR1_OFFSET
#define CCM_CCGR5_OFFSET 0x7c
#define CCM_CCGR5   CCM_IPS_BASE_ADDR+CCM_CCGR5_OFFSET

#define EPIT1_IPS_BASE_ADDR	0x20D0000
#define EPIT_BASE_ADDR          EPIT1_IPS_BASE_ADDR

#define HW_ANADIG_USB1_PLL_480_CTRL_RW  (ANATOP_IPS_BASE_ADDR+0x10) // Anadig 480MHz PLL Control0 Register
//#define USB_OTG_BASE_ADDR	0x02184000

#define SOC_MEM_MAP_ENTRY(name)	\
	unsigned int soc_mem_map_##name##_base[6] = {name##_BASE_ADDR_MX6DQ, \
			name##_BASE_ADDR_MX6SDL,	\
			name##_BASE_ADDR_MX6SL,	\
			name##_BASE_ADDR_MX6SX,	\
			name##_BASE_ADDR_MX6UL,	\
			name##_BASE_ADDR_MX6ULL	\
			};

#define PLAT_MODULE_BASE_INIT(name, cpu)	\
	regs_##name##_base = soc_mem_map_##name##_base[(cpu)] ;

#define IOMUX_CFG_TBL_DEF(name)	\
	unsigned int iomux_##name##_cfg_tbl[6] = { \
	        (unsigned int)name##_mx6dq_iomux_cfg, \
			(unsigned int)name##_mx6dl_iomux_cfg,	\
			(unsigned int)name##_mx6sl_iomux_cfg,	\
			(unsigned int)name##_mx6sx_iomux_cfg,	\
			(unsigned int)name##_mx6ul_iomux_cfg,	\
			(unsigned int)name##_mx6ull_iomux_cfg	\
			};

#define PLAT_MODULE_IOMUX_CFG(name, cpu)   \
    iomux_##name##_cfg = iomux_##name##_cfg_tbl[(cpu)] ;

#define NEED_MODULE(name)	\
	extern unsigned int regs_##name##_base; \
	extern unsigned int soc_mem_map_##name##_base[];

#endif

