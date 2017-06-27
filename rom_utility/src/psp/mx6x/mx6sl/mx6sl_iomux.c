
#include "io.h"
#include "mx6sl_iomux.h"

static iomux_cfg_t mx6sl_usdhc1_pad[] = {
    MX6SL_PAD_SD1_CLK,
    MX6SL_PAD_SD1_CMD,
    MX6SL_PAD_SD1_DAT0,
    MX6SL_PAD_SD1_DAT1,
    MX6SL_PAD_SD1_DAT2,
    MX6SL_PAD_SD1_DAT3,
    0,
};

static iomux_cfg_t mx6sl_usdhc2_pad[] = {
    MX6SL_PAD_SD2_CLK,
    MX6SL_PAD_SD2_CMD,
    MX6SL_PAD_SD2_DAT0,
    MX6SL_PAD_SD2_DAT1,
    MX6SL_PAD_SD2_DAT2,
    MX6SL_PAD_SD2_DAT3,
    0,
};

static iomux_cfg_t mx6sl_usdhc3_pad[] = {
    MX6SL_PAD_SD3_CLK,
    MX6SL_PAD_SD3_CMD,
    MX6SL_PAD_SD3_DAT0,
    MX6SL_PAD_SD3_DAT1,
    MX6SL_PAD_SD3_DAT2,
    MX6SL_PAD_SD3_DAT3,
    0,
};


unsigned int USDHC_mx6sl_iomux_cfg[] = {
    (unsigned int)mx6sl_usdhc1_pad,
    (unsigned int)mx6sl_usdhc2_pad,
    (unsigned int)mx6sl_usdhc3_pad,
    0,
};

/* nand iomux */
iomux_cfg_t NAND_mx6sl_iomux_cfg[] = {
	0,
};

