#include "io.h"
#include "debug.h"
#include "soc_info.h"
#include "platform.h"
#include "timer/timer.h"
//#include "../apps/log_parser.h"
#include "quadspi/qspi.h"
#include "iomux_setting.h"
#include "iomuxc/iomuxc.h"


#define ANATOP_BASE_ADDR 0x20C8000

NEED_MODULE(OCOTP)
    NEED_MODULE(IOMUXC)
    NEED_MODULE(USDHC1)
    NEED_MODULE(USDHC2)
    NEED_MODULE(USDHC3)
    NEED_MODULE(USDHC4)
    NEED_MODULE(WEIM_CS)
    NEED_MODULE(APBH_DMA)
    NEED_MODULE(GPMI)
    NEED_MODULE(BCH)
    NEED_MODULE(USB)

extern uint32_t iomux_NAND_cfg;
extern unsigned int iomux_NAND_cfg_tbl[3];
extern void hapi_mmu_disable(void);
extern unsigned int QSPI_mx6sx_iomux_cfg[];
extern unsigned int QSPI_mx6ul_iomux_cfg[];
extern unsigned int QSPI_mx6ull_iomux_cfg[];

extern void usb_init(cpu_type_e cpu);

extern chip_e chip;

static int mx6_get_cpu_type(void)
{
    unsigned int cpu_type = *(unsigned int *)(ANATOP_BASE_ADDR + 0x280);
    unsigned int cpu_to = *(unsigned int *)(0x48);

    cpu_type >>= 16;
    if (cpu_type == 0x60) {
        printf("i.MX6SL TO %d.%d\n", (cpu_to & 0xf0) >> 4, cpu_to & 0x0f);
        return (CHIP_MX6SL_TO1_0 + (cpu_to & 0x0f));
    }

    cpu_type = *(unsigned int *)(ANATOP_BASE_ADDR + 0x260);
    cpu_type >>= 16;
    printf("Chip: ");

    if (cpu_type == 0x63) {
        printf("i.MX6DQ TO %d.%d\n", (cpu_to & 0xf0) >> 4, cpu_to & 0x0f);
        return (CHIP_MX6DQ_TO1_0 + (cpu_to & 0x0f) + (cpu_to & 0xf0) - 0x10);
    } else if (cpu_type == 0x61) {
        printf("i.MX6SDL TO %d.%d\n", (cpu_to & 0xf0) >> 4, cpu_to & 0x0f);
        return (CHIP_MX6SDL_TO1_0 + (cpu_to & 0x0f) + (cpu_to & 0xf0) - 0x10);
    } else if (cpu_type == 0x62) {
        cpu_to = *(unsigned int *)(0x80);
        printf("i.MX6SX TO %d.%d\n", (cpu_to & 0xf0) >> 4, cpu_to & 0x0f);
        return (CHIP_MX6SX_TO1_0 + (cpu_to & 0x0f) + (cpu_to & 0xf0) - 0x10);
    } else if (cpu_type == 0x64) {
        cpu_to = *(unsigned int *)(0x80);
        printf("i.MX6UL TO %d.%d\n", (cpu_to & 0xf0) >> 4, cpu_to & 0x0f);
        return (CHIP_MX6UL_TO1_0 + (cpu_to & 0x0f) + (cpu_to & 0xf0) - 0x10);
    } else if (cpu_type == 0x65) {
        cpu_to = *(unsigned int *)(0x80);
        printf("i.MX6ULL TO %d.%d\n", (cpu_to & 0xf0) >> 4, cpu_to & 0x0f);
        return (CHIP_MX6ULL_TO1_0 + (cpu_to & 0x0f) + (cpu_to & 0xf0) - 0x10);
    } else if (cpu_type == 0x67) {
        cpu_to = *(unsigned int *)(0x80);
        printf("i.MX6SLL TO %d.%d\n", (cpu_to & 0xf0) >> 4, cpu_to & 0x0f);
        return (CHIP_MX6SLL_TO1_0 + (cpu_to & 0x0f) + (cpu_to & 0xf0) - 0x10);
	} 
	else {
        printf("Unknown CPU type: %x\n", cpu_type);
    }

    return CHIP_MAX;
}

int get_chip_type(void)
{
    return mx6_get_cpu_type();
}

int get_usdhc_port(uint32_t base)
{
    int port;

    if (regs_USDHC1_base == base)
        port = USDHC_PORT1;
    else if (regs_USDHC2_base == base)
        port = USDHC_PORT2;
    else if (regs_USDHC3_base == base)
        port = USDHC_PORT3;
    else if (regs_USDHC4_base == base)
        port = USDHC_PORT4;
    else
        port = USDHC_NUMBER_PORTS;

    return port;
}

uint32_t get_usdhc_base(usdhc_port_e port)
{
    uint32_t base;

    if (USDHC_PORT1 == port)
        base = regs_USDHC1_base;
    else if (USDHC_PORT2 == port)
        base = regs_USDHC2_base;
    else if (USDHC_PORT3 == port)
        base = regs_USDHC3_base;
    else if (USDHC_PORT4 == port)
        base = regs_USDHC4_base;
    else
        base = 0;

    return base;
}

uint32_t psp_get_spi_base(spi_port_e port)
{
    uint32_t spi_base[] = {
        0x02008000,
        0x0200c000,
        0x02010000,
        0x02014000
    };

    /* TODO */

    return spi_base[port];
}

uint32_t psp_get_i2c_base(i2c_port_e port)
{
    uint32_t i2c_base[] = {
        0x021a0000,
        0x021a4000,
        0x021a8000
    };

    return i2c_base[port];
}

uint32_t qspi_get_instance(uint32_t index)
{
    if (QUAD_SPI_1 == index)
        return QSPI1_BASE_ADDR_MX6SX;
    else if (QUAD_SPI_2 == index)
        return QSPI2_BASE_ADDR_MX6SX;
    else
        return NULL;
}

uint32_t qspi_get_index(uint32_t instance)
{

    if (QSPI1_BASE_ADDR_MX6SX == instance)
        return QUAD_SPI_1;
    else if (QSPI2_BASE_ADDR_MX6SX == instance)
        return QUAD_SPI_2;
    else
        return QUAD_SPI_INVAL;
}

uint32_t qspi_map_memory_base(uint32_t instance)
{
    if (QSPI1_BASE_ADDR_MX6SX == (uint32_t)instance)
        return QSPI0_ARB_BASE_ADDR_MX6SX;
    else if (QSPI2_BASE_ADDR_MX6SX == (uint32_t)instance)
        return QSPI1_ARB_BASE_ADDR_MX6SX;
    else
        return 0;
}

#if 0
static void psp_nand_iomux_config(void)
{
	module_iomux_configuration_setup((iomux_cfg_t *)iomux_NAND_cfg);

#if 1
    // for 6ull nand test only
    // call interface_iomux_configuration @ 0x00006589

    f_v_v interface_iomux_configuration = (f_v_v)0x00006589;

    interface_iomux_configuration();
#endif
}
#endif

int platform_init(chip_e chip)
{
    cpu_type_e cpu;

    if (chip >= CHIP_MX6DQ_TO1_0 && chip <= CHIP_MX6DQ_MAX) {
        cpu = CPU_TYPE_MX6DQ;
    } else if (chip >= CHIP_MX6SDL_TO1_0 && chip <= CHIP_MX6SDL_MAX) {
        cpu = CPU_TYPE_MX6SDL;
    } else if (chip >= CHIP_MX6SL_TO1_0 && chip <= CHIP_MX6SL_MAX) {
        cpu = CPU_TYPE_MX6SL;
    } else if (chip >= CHIP_MX6SX_TO1_0 && chip <= CHIP_MX6SX_MAX) {
        cpu = CPU_TYPE_MX6SX;
    } else if (chip >= CHIP_MX6UL_TO1_0 && chip <= CHIP_MX6UL_MAX) {
        cpu = CPU_TYPE_MX6UL;
	} else if (chip >= CHIP_MX6ULL_TO1_0 && chip <= CHIP_MX6ULL_MAX) {
        cpu = CPU_TYPE_MX6ULL;
	} else {
		cpu = CPU_TYPE_MX6SLL;
    }

    /* EPIT1 clock not gate-on by default on Pele, may affect DCD. ??? */
    if (CPU_TYPE_MX6SX == cpu) {
#define CCM_BASE 0x020c4000
#define CCGR1	(CCM_BASE + 0x6C)
#define CCGR_CG6	0x3000
#define IOMUX_GPR11	(0x20e402c)
        writel(readl(CCGR1) | CCGR_CG6, CCGR1);

	/* Configure L2 as ocram for bigger memory*/
	/* To call arm code. Not sure how the GCC supports this. */
	asm volatile ("	ldr r0, =hapi_mmu_disable\nblx r0" ::);
	writel(readl(IOMUX_GPR11) | 0x02, IOMUX_GPR11);
    }

    //init_clock(32768);
    #if 1
	PLAT_MODULE_BASE_INIT(USDHC1, cpu);
    PLAT_MODULE_BASE_INIT(OCOTP, cpu);
    PLAT_MODULE_BASE_INIT(IOMUXC, cpu);
    PLAT_MODULE_BASE_INIT(USDHC1, cpu);
    PLAT_MODULE_BASE_INIT(USDHC2, cpu);
    PLAT_MODULE_BASE_INIT(USDHC3, cpu);
    PLAT_MODULE_BASE_INIT(USDHC4, cpu);
    PLAT_MODULE_BASE_INIT(WEIM_CS, cpu);
    PLAT_MODULE_IOMUX_CFG(NAND, cpu);
	//psp_nand_iomux_config();

    PLAT_MODULE_BASE_INIT(APBH_DMA, cpu);
    PLAT_MODULE_BASE_INIT(GPMI, cpu);

    PLAT_MODULE_BASE_INIT(BCH, cpu);

    PLAT_MODULE_BASE_INIT(USB, cpu);
    #endif 
#ifdef USB_PRINT
	usb_init(cpu);
#endif

    return 0;
}

void psp_qspi_iomux_config(uint32_t index)
{
    chip_e ctype = CHIP_MAX;
    uint32_t pad_cfg_addr = 0;

    ctype = get_chip_type();
    if ((ctype >= CHIP_MX6SX_TO1_0) && (ctype < CHIP_MX6SX_MAX)) {
        pad_cfg_addr = *((uint32_t *)QSPI_mx6sx_iomux_cfg + index);
    } else if ((ctype >= CHIP_MX6UL_TO1_0) && (ctype < CHIP_MX6UL_MAX)) {
        pad_cfg_addr = *((uint32_t *)QSPI_mx6ul_iomux_cfg + index);
	} else if ((ctype >= CHIP_MX6ULL_TO1_0) && (ctype < CHIP_MX6ULL_MAX)) {
		pad_cfg_addr = *((uint32_t *)QSPI_mx6ull_iomux_cfg + index);
    } else {
        DEBUG_LOG("psp_qspi_iomux_config : only sx and 6ul and 6ull support QSPI\n");
        return;
    }

    module_iomux_configuration_setup((iomux_cfg_t *)pad_cfg_addr);
}


