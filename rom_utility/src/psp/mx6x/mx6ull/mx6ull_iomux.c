
#include "io.h"
#include "mx6ull_iomux.h"


/*============== QSPI1 pin mux and pad settings table =================*/
static iomux_cfg_t mx6ull_qspi1_pad[] = {
    MX6ULL_PAD_QSPI1A_CLK,
    MX6ULL_PAD_QSPI1A_CS0,
    MX6ULL_PAD_QSPI1A_CS1,
    MX6ULL_PAD_QSPI1A_DAT0,
    MX6ULL_PAD_QSPI1A_DAT1,
    MX6ULL_PAD_QSPI1A_DAT2,
    MX6ULL_PAD_QSPI1A_DAT3,
    MX6ULL_PAD_QSPI1A_DQS,
    MX6ULL_PAD_QSPI1B_CLK,
    MX6ULL_PAD_QSPI1B_CS0,
    MX6ULL_PAD_QSPI1B_CS1,
    MX6ULL_PAD_QSPI1B_DAT0,
    MX6ULL_PAD_QSPI1B_DAT1,
    MX6ULL_PAD_QSPI1B_DAT2,
    MX6ULL_PAD_QSPI1B_DAT3,
    MX6ULL_PAD_QSPI1B_DQS,
    0,
};

/*============== 6UL QSPI pin mux and pad settings table =================*/
unsigned int QSPI_mx6ull_iomux_cfg[] = {
    (unsigned int)mx6ull_qspi1_pad,
};


/* nand iomux */
iomux_cfg_t NAND_mx6ull_iomux_cfg_bad[] = {
    MX6ULL_PAD_RE_B_ADDR,
    MX6ULL_PAD_WE_B_ADDR,
    MX6ULL_PAD_DATA00_ADDR,
    MX6ULL_PAD_DATA01_ADDR,
    MX6ULL_PAD_DATA02_ADDR,
    MX6ULL_PAD_DATA03_ADDR,
    MX6ULL_PAD_DATA04_ADDR,
    MX6ULL_PAD_DATA05_ADDR,
    MX6ULL_PAD_DATA06_ADDR,
    MX6ULL_PAD_DATA07_ADDR,
    MX6ULL_PAD_ALE_ADDR,
    MX6ULL_PAD_WP_B_ADDR,
    MX6ULL_PAD_READY_B_ADDR,
    MX6ULL_PAD_CE0_B_ADDR,
    MX6ULL_PAD_CE1_B_ADDR,
    MX6ULL_PAD_CLE_ADDR,
    MX6ULL_PAD_DQS_ADDR,
};

iomux_cfg_t NAND_mx6ull_iomux_cfg[] = {
    0
};
