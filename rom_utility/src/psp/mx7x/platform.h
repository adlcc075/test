#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "soc_info.h"
#include "soc_mem_maps/mx7d_soc_memory_map.h"

typedef enum {
    USDHC_PORT1 = 0,
    USDHC_PORT2 = 1,
    USDHC_PORT3 = 2,
    USDHC_PORT4 = 3,
    USDHC_NUMBER_PORTS
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

/*
#define ANATOP_IPS_BASE_ADDR      	ANATOP_BASE_ADDR_MX7D

#define IP2APB_USBPHY1_BASE_ADDR  IP2APB_USBPHY1_BASE_ADDR_MX7D
#define IP2APB_USBPHY2_BASE_ADDR  IP2APB_USBPHY2_BASE_ADDR_MX7D

#define CCM_IPS_BASE_ADDR	CCM_BASE_ADDR_MX7D
*/
#define SOC_MEM_MAP_ENTRY(name)	\
	unsigned int soc_mem_map_##name##_base[] = { \
			name##_BASE_ADDR_MX7D,	\
			};

#define PLAT_MODULE_BASE_INIT(name, cpu)	\
	regs_##name##_base = soc_mem_map_##name##_base[(cpu)] ;

#define IOMUX_CFG_TBL_DEF(name)	\
	unsigned int iomux_##name##_cfg_tbl[] = { \
			(unsigned int)name##_mx7d_iomux_cfg,	\
			};

#define PLAT_MODULE_IOMUX_CFG(name, cpu)   \
    iomux_##name##_cfg = iomux_##name##_cfg_tbl[(cpu)] ;

#define NEED_MODULE(name)	\
	extern unsigned int regs_##name##_base; \
	extern unsigned int soc_mem_map_##name##_base[];

#endif

