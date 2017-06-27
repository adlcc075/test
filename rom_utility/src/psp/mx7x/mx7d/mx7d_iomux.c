#include "io.h"
#include "mx7d_iomux.h"

static iomux_cfg_t mx7d_usdhc2_pad[] = {
    MX7D_PAD_SD2_CLK,
    MX7D_PAD_SD2_CMD,
    MX7D_PAD_SD2_DAT0,
    MX7D_PAD_SD2_DAT1,
    MX7D_PAD_SD2_DAT2,
    MX7D_PAD_SD2_DAT3,
    0,
};

static iomux_cfg_t mx7d_usdhc3_pad[] = {
    MX7D_PAD_SD3_CLK,
    MX7D_PAD_SD3_CMD,
    MX7D_PAD_SD3_DAT0,
    MX7D_PAD_SD3_DAT1,
    MX7D_PAD_SD3_DAT2,
    MX7D_PAD_SD3_DAT3,
    0,
};

unsigned int USDHC_mx7d_iomux_cfg[] = {
    (unsigned int)0,
    (unsigned int)mx7d_usdhc2_pad,
    (unsigned int)mx7d_usdhc3_pad,
};

/*============== QSPI1 pin mux and pad settings table =================*/
static iomux_cfg_t mx7d_qspi1_pad[] = {
    MX7D_PAD_QSPI1A_CLK,
    MX7D_PAD_QSPI1A_CS0,
    MX7D_PAD_QSPI1A_CS1,
    MX7D_PAD_QSPI1A_DAT0,
    MX7D_PAD_QSPI1A_DAT1,
    MX7D_PAD_QSPI1A_DAT2,
    MX7D_PAD_QSPI1A_DAT3,
    MX7D_PAD_QSPI1A_DQS,
    MX7D_PAD_QSPI1B_CLK,
    MX7D_PAD_QSPI1B_CS0,
    MX7D_PAD_QSPI1B_CS1,
    MX7D_PAD_QSPI1B_DAT0,
    MX7D_PAD_QSPI1B_DAT1,
    MX7D_PAD_QSPI1B_DAT2,
    MX7D_PAD_QSPI1B_DAT3,
    MX7D_PAD_QSPI1B_DQS,
    0,
};

/*============== 6SX QSPI pin mux and pad settings table =================*/
unsigned int QSPI_mx7d_iomux_cfg[] = {
    (unsigned int)mx7d_qspi1_pad,
};


/* nand iomux */
iomux_cfg_t NAND_mx7d_iomux_cfg[] = {
    MX7D_PAD_CE0_B,
    MX7D_PAD_CE1_B,
    MX7D_PAD_CE2_B,
    MX7D_PAD_CE3_B,
	MX7D_PAD_WP_B,
	0,
};

