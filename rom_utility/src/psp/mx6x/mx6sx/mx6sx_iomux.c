
#include "io.h"
#include "mx6sx_iomux.h"

static iomux_cfg_t mx6sx_usdhc2_pad[] = {
    MX6SX_PAD_SD2_CLK,
    MX6SX_PAD_SD2_CMD,
    MX6SX_PAD_SD2_DAT0,
    MX6SX_PAD_SD2_DAT1,
    MX6SX_PAD_SD2_DAT2,
    MX6SX_PAD_SD2_DAT3,
    0,
};

static iomux_cfg_t mx6sx_usdhc3_pad[] = {
    MX6SX_PAD_SD3_CLK,
    MX6SX_PAD_SD3_CMD,
    MX6SX_PAD_SD3_DAT0,
    MX6SX_PAD_SD3_DAT1,
    MX6SX_PAD_SD3_DAT2,
    MX6SX_PAD_SD3_DAT3,
    0,
};

static iomux_cfg_t mx6sx_usdhc4_pad[] = {
    MX6SX_PAD_SD4_CLK,
    MX6SX_PAD_SD4_CMD,
    MX6SX_PAD_SD4_DAT0,
    MX6SX_PAD_SD4_DAT1,
    MX6SX_PAD_SD4_DAT2,
    MX6SX_PAD_SD4_DAT3,
    0,
};

unsigned int USDHC_mx6sx_iomux_cfg[] = {
    (unsigned int)0,
    (unsigned int)mx6sx_usdhc2_pad,
    (unsigned int)mx6sx_usdhc3_pad,
    (unsigned int)mx6sx_usdhc4_pad,
};

/*============== QSPI1 pin mux and pad settings table =================*/
static iomux_cfg_t mx6sx_qspi1_pad[] = {
    MX6SX_PAD_QSPI1A_CLK,
    MX6SX_PAD_QSPI1A_CS0,
    MX6SX_PAD_QSPI1A_CS1,
    MX6SX_PAD_QSPI1A_DAT0,
    MX6SX_PAD_QSPI1A_DAT1,
    MX6SX_PAD_QSPI1A_DAT2,
    MX6SX_PAD_QSPI1A_DAT3,
    MX6SX_PAD_QSPI1A_DQS,
    MX6SX_PAD_QSPI1B_CLK,
    MX6SX_PAD_QSPI1B_CS0,
    MX6SX_PAD_QSPI1B_CS1,
    MX6SX_PAD_QSPI1B_DAT0,
    MX6SX_PAD_QSPI1B_DAT1,
    MX6SX_PAD_QSPI1B_DAT2,
    MX6SX_PAD_QSPI1B_DAT3,
    MX6SX_PAD_QSPI1B_DQS,
    0,
};

/*============== QSPI2 pin mux and pad settings table =================*/
static iomux_cfg_t mx6sx_qspi2_pad[] = {
    MX6SX_PAD_QSPI2A_CLK,
    MX6SX_PAD_QSPI2A_CS0,
    MX6SX_PAD_QSPI2A_CS1,
    MX6SX_PAD_QSPI2A_DAT0,
    MX6SX_PAD_QSPI2A_DAT1,
    MX6SX_PAD_QSPI2A_DAT2,
    MX6SX_PAD_QSPI2A_DAT3,
    MX6SX_PAD_QSPI2A_DQS,
    MX6SX_PAD_QSPI2B_CLK,
    MX6SX_PAD_QSPI2B_CS0,
    MX6SX_PAD_QSPI2B_CS1,
    MX6SX_PAD_QSPI2B_DAT0,
    MX6SX_PAD_QSPI2B_DAT1,
    MX6SX_PAD_QSPI2B_DAT2,
    MX6SX_PAD_QSPI2B_DAT3,
    MX6SX_PAD_QSPI2B_DQS,
    0,
};

/*============== 6SX QSPI pin mux and pad settings table =================*/
unsigned int QSPI_mx6sx_iomux_cfg[] = {
    (unsigned int)mx6sx_qspi1_pad,
    (unsigned int)mx6sx_qspi2_pad,
};


/* nand iomux */
iomux_cfg_t NAND_mx6sx_iomux_cfg[] = {
    MX6SX_PAD_CE0_B,
    MX6SX_PAD_CE1_B,
	0,
};

