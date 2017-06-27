
#include "io.h"
#include "mx6sll_iomux.h"

static iomux_cfg_t mx6sll_usdhc1_pad[] = {
    MX6SLL_PAD_SD1_CLK,
    MX6SLL_PAD_SD1_CMD,
    MX6SLL_PAD_SD1_DAT0,
    MX6SLL_PAD_SD1_DAT1,
    MX6SLL_PAD_SD1_DAT2,
    MX6SLL_PAD_SD1_DAT3,
    0,
};

static iomux_cfg_t mx6sll_usdhc2_pad[] = {
    MX6SLL_PAD_SD2_CLK,
    MX6SLL_PAD_SD2_CMD,
    MX6SLL_PAD_SD2_DAT0,
    MX6SLL_PAD_SD2_DAT1,
    MX6SLL_PAD_SD2_DAT2,
    MX6SLL_PAD_SD2_DAT3,
    0,
};

static iomux_cfg_t mx6sll_usdhc3_pad[] = {
    MX6SLL_PAD_SD3_CLK,
    MX6SLL_PAD_SD3_CMD,
    MX6SLL_PAD_SD3_DAT0,
    MX6SLL_PAD_SD3_DAT1,
    MX6SLL_PAD_SD3_DAT2,
    MX6SLL_PAD_SD3_DAT3,
    0,
};


unsigned int USDHC_mx6sll_iomux_cfg[] = {
    (unsigned int)mx6sll_usdhc1_pad,
    (unsigned int)mx6sll_usdhc2_pad,
    (unsigned int)mx6sll_usdhc3_pad,
    0,
};

/* nand iomux */
iomux_cfg_t NAND_mx6sll_iomux_cfg[] = {
	0,
};

