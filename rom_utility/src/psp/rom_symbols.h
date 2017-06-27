#ifndef _ROM_SYMBOLS_H_
#define _ROM_SYMBOLS_H_

#include "io.h"
#include "soc_info.h"

typedef enum {
    ROM_SYM_f_SETUP_BOOT_SELECTION,
    ROM_SYM_f_device_init,
    ROM_SYM_f_device_read,
    ROM_SYM_v_SBMR_SHADOW,
    ROM_SYM_f_ecspi_init,
    ROM_SYM_f_card_init,
    ROM_SYM_v_hapi_src_reg,
    ROM_SYM_v_port_esdhc,
    ROM_SYM_f_cfg_qspi_clock,
    ROM_SYM_f_GPT_init,
    ROM_SYM_f_GPT_delay,
    ROM_SYM_f_GPT_status,
    ROM_SYM_f_GPT_reset_delay,
    ROM_SYM_f_GPT_disable,
} ROM_SYM_e;

u32 psp_rom_get_symble(ROM_SYM_e sym, chip_e chip);

#endif
