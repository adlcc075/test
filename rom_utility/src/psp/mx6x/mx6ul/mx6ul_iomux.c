
#include "io.h"
#include "mx6ul_iomux.h"


/*============== QSPI1 pin mux and pad settings table =================*/
static iomux_cfg_t mx6ul_qspi1_pad[] = {
    MX6UL_PAD_QSPI1A_CLK,
    MX6UL_PAD_QSPI1A_CS0,
    MX6UL_PAD_QSPI1A_CS1,
    MX6UL_PAD_QSPI1A_DAT0,
    MX6UL_PAD_QSPI1A_DAT1,
    MX6UL_PAD_QSPI1A_DAT2,
    MX6UL_PAD_QSPI1A_DAT3,
    MX6UL_PAD_QSPI1A_DQS,
    MX6UL_PAD_QSPI1B_CLK,
    MX6UL_PAD_QSPI1B_CS0,
    MX6UL_PAD_QSPI1B_CS1,
    MX6UL_PAD_QSPI1B_DAT0,
    MX6UL_PAD_QSPI1B_DAT1,
    MX6UL_PAD_QSPI1B_DAT2,
    MX6UL_PAD_QSPI1B_DAT3,
    MX6UL_PAD_QSPI1B_DQS,
    0,
};

/*============== 6UL QSPI pin mux and pad settings table =================*/
unsigned int QSPI_mx6ul_iomux_cfg[] = {
    (unsigned int)mx6ul_qspi1_pad,
};


/* nand iomux */
iomux_cfg_t NAND_mx6ul_iomux_cfg[] = {
	0,
};

