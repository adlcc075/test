
#include <io.h>
#include "rom_symbols.h"
#include "boot_cfg.h"

u32 boot_cfg_override(BOOT_DEV_e dev, u32 port, chip_e chip, u8 *para)
{
    u32 ret = 0;
    u32 *hapi_src_reg_addr = 0;

    hapi_src_reg_addr = (u32 *)psp_rom_get_symble(ROM_SYM_v_hapi_src_reg, chip);
    if (!hapi_src_reg_addr) {
        printf("get hapi_src_reg address failure\n");
        return 1;
    }
	printf("hapi_src_reg_addr=0x%x\n",hapi_src_reg_addr);

    switch  (dev) {
    case    WEIMNOR_BOOT:
            break;

    case    QSPINOR_BOOT:
            break;

    case    SPINOR_BOOT:
            break;

    case    SD_BOOT:
            break;

    case    MMC_BOOT:
            switch  (chip) {
            case    CHIP_MX6DQ_TO1_0:
            case    CHIP_MX6DQ_TO1_1:
            case    CHIP_MX6DQ_TO1_2:
            case    CHIP_MX6DQ_TO1_3:
            case    CHIP_MX6DQ_TO1_4:
            case    CHIP_MX6DQ_TO1_5:
                    break;

            case    CHIP_MX6SDL_TO1_0:
            case    CHIP_MX6SDL_TO1_1:
            case    CHIP_MX6SDL_TO1_2:
                    break;

            case    CHIP_MX6SL_TO1_0:
            case    CHIP_MX6SL_TO1_1:
            case    CHIP_MX6SL_TO1_2:
                    break;

            case    CHIP_MX6SX_TO1_0:
            case    CHIP_MX6SX_TO1_1:
                    *hapi_src_reg_addr = (0x00000060 | ((port - 1) << 11));
                    break;

            case    CHIP_MX7D_TO1_0:
                    *hapi_src_reg_addr = (0x00002000 | ((port - 1) << 10));
                    break;

            case    CHIP_MX6UL_TO1_0:
                    *hapi_src_reg_addr = (0x00000060 | ((port - 1) << 11));
                    break;

			case    CHIP_MX6ULL_TO1_0:
					*hapi_src_reg_addr = (0x00000060 | ((port - 1) << 11));
					break;

			case	CHIP_MX6SLL_TO1_0:
					*hapi_src_reg_addr = (0x00000060 | ((port - 1) << 11));
					printf("hapi_src_reg_addr_2=0x%x\n",hapi_src_reg_addr);
					break;

            case    CHIP_MX6DQ_MAX:
            case    CHIP_MX6SDL_MAX:
            case    CHIP_MX6SL_MAX:
            case    CHIP_MX6SX_MAX:
            case    CHIP_MX7D_MAX:
            case    CHIP_MX6UL_MAX:
			case    CHIP_MX6ULL_MAX:
			case    CHIP_MX6SLL_MAX:
            case    CHIP_MAX:
                    ret = 1;
                    break;
            default:
                    ret = 1;
                    break;
            }
            break;

    case    NAND_BOOT:
            break;

    default:
            ret = 1;
            break;
    }

    return ret;
}


