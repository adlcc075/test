
#include "io.h"
#include "mx6dl_iomux.h"

static iomux_cfg_t mx6dl_usdhc1_pad[] = {
    MX6DL_PAD_SD1_CLK,
    MX6DL_PAD_SD1_CMD,
    MX6DL_PAD_SD1_DAT0,
    MX6DL_PAD_SD1_DAT1,
    MX6DL_PAD_SD1_DAT2,
    MX6DL_PAD_SD1_DAT3,
    0,
};

static iomux_cfg_t mx6dl_usdhc2_pad[] = {
    MX6DL_PAD_SD2_CLK,
    MX6DL_PAD_SD2_CMD,
    MX6DL_PAD_SD2_DAT0,
    MX6DL_PAD_SD2_DAT1,
    MX6DL_PAD_SD2_DAT2,
    MX6DL_PAD_SD2_DAT3,
    0,
};

iomux_cfg_t mx6dl_usdhc3_pad[] = {
    MX6DL_PAD_SD3_CLK,
    MX6DL_PAD_SD3_CMD,
    MX6DL_PAD_SD3_DAT0,
    MX6DL_PAD_SD3_DAT1,
    MX6DL_PAD_SD3_DAT2,
    MX6DL_PAD_SD3_DAT3,
    0,
};

static iomux_cfg_t mx6dl_usdhc4_pad[] = {
    MX6DL_PAD_SD4_CLK,
    MX6DL_PAD_SD4_CMD,
    MX6DL_PAD_SD4_DAT0,
    MX6DL_PAD_SD4_DAT1,
    MX6DL_PAD_SD4_DAT2,
    MX6DL_PAD_SD4_DAT3,
    0,
};

unsigned int USDHC_mx6dl_iomux_cfg[] = {
    (unsigned int)mx6dl_usdhc1_pad,
    (unsigned int)mx6dl_usdhc2_pad,
    (unsigned int)mx6dl_usdhc3_pad,
    (unsigned int)mx6dl_usdhc4_pad,
};

/* nand iomux */
iomux_cfg_t NAND_mx6dl_iomux_cfg[] = {
	0,
};

