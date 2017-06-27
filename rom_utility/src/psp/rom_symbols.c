#include <io.h>
#include "rom_symbols.h"


u32 psp_rom_get_symble(ROM_SYM_e sym, chip_e chip)
{
    u32 res = 0;

    switch  (sym)   {
    case    ROM_SYM_f_SETUP_BOOT_SELECTION:
            switch  (chip) {
            case    CHIP_MX6DQ_TO1_0:
            case    CHIP_MX6DQ_TO1_1:
            case    CHIP_MX6DQ_TO1_2:
            case    CHIP_MX6DQ_TO1_3:
            case    CHIP_MX6DQ_TO1_4:
            case    CHIP_MX6DQ_TO1_5:
					res = 0x000020f7;
					break;
			case	CHIP_MX6DQ_TO2_0:
                    break;

            case    CHIP_MX6SDL_TO1_0:
            case    CHIP_MX6SDL_TO1_1:
            case    CHIP_MX6SDL_TO1_2:
            case    CHIP_MX6SDL_TO1_3:
                    break;

            case    CHIP_MX6SL_TO1_0:
            case    CHIP_MX6SL_TO1_1:
            case    CHIP_MX6SL_TO1_2:
                    break;

            case    CHIP_MX6SX_TO1_0:
                    res = 0x25cb;
                    break;

            case    CHIP_MX6SX_TO1_1:
                    res = 0x2619;
                    break;

            case    CHIP_MX7D_TO1_0:
                    res = 0x6a4f;
                    break;

            case    CHIP_MX6UL_TO1_0:
                    res = 0x29cd;
                    break;

			case    CHIP_MX6ULL_TO1_0:
					res = 0x29e5;
					break;

			case    CHIP_MX6SLL_TO1_0:
				    res = 0x281f;//pu_irom_setup_boot_selection
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
                    break;
            }
            break;

    case    ROM_SYM_f_device_init:
            switch  (chip)  {
            case    CHIP_MX6DQ_TO1_0:
            case    CHIP_MX6DQ_TO1_1:
            case    CHIP_MX6DQ_TO1_2:
            case    CHIP_MX6DQ_TO1_3:
            case    CHIP_MX6DQ_TO1_4:
            case    CHIP_MX6DQ_TO1_5:
                    res = *(u32 *)0x009008b4;	// device_init is the first word of boot driver selection
                    break;
			case	CHIP_MX6DQ_TO2_0:
                    break;

            case    CHIP_MX6SDL_TO1_0:
            case    CHIP_MX6SDL_TO1_1:
            case    CHIP_MX6SDL_TO1_2:
            case    CHIP_MX6SDL_TO1_3:
                    break;

            case    CHIP_MX6SL_TO1_0:
            case    CHIP_MX6SL_TO1_1:
            case    CHIP_MX6SL_TO1_2:
                    break;

            case    CHIP_MX6SX_TO1_0:
                    res = *(u32 *)0x900b50;	// device_init is the first word of boot driver selection
                    break;

            case    CHIP_MX6SX_TO1_1:
                    res = *(u32 *)0x900c44;
                    break;

            case    CHIP_MX7D_TO1_0:
                    res = *(u32 *)0x9090e0;
                    break;

            case    CHIP_MX6UL_TO1_0:
                    res = *(u32 *)0x900c80;
                    break;

			case    CHIP_MX6ULL_TO1_0:
					res = *(u32 *)0x900c60;
					break;

			case    CHIP_MX6SLL_TO1_0:
				    res = *(u32 *)0x00900b44; // device_init is the first word of boot driver selection
				                             // boot_driver_selection
			
            case    CHIP_MX6DQ_MAX:
            case    CHIP_MX6SDL_MAX:
            case    CHIP_MX6SL_MAX:
            case    CHIP_MX6SX_MAX:
            case    CHIP_MX7D_MAX:
            case    CHIP_MX6UL_MAX:
            case    CHIP_MX6ULL_MAX:
            case    CHIP_MX6SLL_MAX:
            case    CHIP_MAX:
                    break;
            }
            break;

    case    ROM_SYM_v_SBMR_SHADOW:
            switch  (chip)  {
            case    CHIP_MX6DQ_TO1_0:
            case    CHIP_MX6DQ_TO1_1:
            case    CHIP_MX6DQ_TO1_2:
            case    CHIP_MX6DQ_TO1_3:
            case    CHIP_MX6DQ_TO1_4:
            case    CHIP_MX6DQ_TO1_5:
                    res = 0x009024b4;
                    break;
			case	CHIP_MX6DQ_TO2_0:
                    break;

            case    CHIP_MX6SDL_TO1_0:
            case    CHIP_MX6SDL_TO1_1:
            case    CHIP_MX6SDL_TO1_2:
            case    CHIP_MX6SDL_TO1_3:
                    break;

            case    CHIP_MX6SL_TO1_0:
            case    CHIP_MX6SL_TO1_1:
            case    CHIP_MX6SL_TO1_2:
                    break;

            case    CHIP_MX6SX_TO1_0:
                    res = 0x900a28;
                    break;

            case    CHIP_MX6SX_TO1_1:
                    res = 0x900a28;
                    break;

            case    CHIP_MX7D_TO1_0:
                    res = 0x9074b0;
                    break;

            case    CHIP_MX6UL_TO1_0:
                    res = 0x900a2c;
                    break;

			case    CHIP_MX6ULL_TO1_0:
					res = 0x900a2c;
					break;

            case    CHIP_MX6SLL_TO1_0:
                    res = 0x4d45; //get persist sbmr value ?
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
                    break;
            }
            break;

    case    ROM_SYM_f_device_read:
            switch  (chip)  {
            case    CHIP_MX6DQ_TO1_0:
            case    CHIP_MX6DQ_TO1_1:
            case    CHIP_MX6DQ_TO1_2:
            case    CHIP_MX6DQ_TO1_3:
            case    CHIP_MX6DQ_TO1_4:
            case    CHIP_MX6DQ_TO1_5:
                    res = *(u32 *)(0x009008b4 + 4);	// device_init is the first word of boot driver selection
                    break;
			case	CHIP_MX6DQ_TO2_0:
                    break;

            case    CHIP_MX6SDL_TO1_0:
            case    CHIP_MX6SDL_TO1_1:
            case    CHIP_MX6SDL_TO1_2:
            case    CHIP_MX6SDL_TO1_3:
                    break;

            case    CHIP_MX6SL_TO1_0:
            case    CHIP_MX6SL_TO1_1:
            case    CHIP_MX6SL_TO1_2:
                    break;

            case    CHIP_MX6SX_TO1_0:
                    res = *(u32 *)(0x900b50 + 4);	// device_init is the first word of boot driver selection
                    break;

            case    CHIP_MX6SX_TO1_1:
                    res = *(u32 *)(0x900c44 + 4);
                    break;

            case    CHIP_MX7D_TO1_0:
                    res = *(u32 *)(0x9090e0 + 4);
                    break;

            case    CHIP_MX6UL_TO1_0:
                    res = *(u32 *)(0x900c80 + 4);
                    break;

			case    CHIP_MX6ULL_TO1_0:
					res = *(u32 *)(0x900c60 + 4);
					break;

			case    CHIP_MX6SLL_TO1_0:
				    res = *(u32 *)(0x00900b44 + 4); // device_init is the first word of boot driver selection

            case    CHIP_MX6DQ_MAX:
            case    CHIP_MX6SDL_MAX:
            case    CHIP_MX6SL_MAX:
            case    CHIP_MX6SX_MAX:
            case    CHIP_MX7D_MAX:
            case    CHIP_MX6UL_MAX:
            case    CHIP_MX6ULL_MAX:
            case    CHIP_MX6SLL_MAX:
			case    CHIP_MAX:
                    break;
            }
            break;

    case    ROM_SYM_f_ecspi_init:
            switch  (chip)  {
            case    CHIP_MX6DQ_TO1_0:
            case    CHIP_MX6DQ_TO1_1:
            case    CHIP_MX6DQ_TO1_2:
            case    CHIP_MX6DQ_TO1_3:
            case    CHIP_MX6DQ_TO1_4:
            case    CHIP_MX6DQ_TO1_5:
                    res = 0x00005ea9;
                    break;
			case	CHIP_MX6DQ_TO2_0:
                    break;

            case    CHIP_MX6SDL_TO1_0:
            case    CHIP_MX6SDL_TO1_1:
            case    CHIP_MX6SDL_TO1_2:
            case    CHIP_MX6SDL_TO1_3:
                    break;

            case    CHIP_MX6SL_TO1_0:
            case    CHIP_MX6SL_TO1_1:
            case    CHIP_MX6SL_TO1_2:
                    break;

            case    CHIP_MX6SX_TO1_0:
                    res = 0x000050a1;
                    break;

            case    CHIP_MX6SX_TO1_1:
                    res = 0x00005191;
                    break;

            case    CHIP_MX7D_TO1_0:
                    res = 0x00006171;
                    break;

            case    CHIP_MX6UL_TO1_0:
                    res = 0x00004e45;
                    break;

			case    CHIP_MX6ULL_TO1_0:
					res = 0x00004e75;
					break;

            case    CHIP_MX6SLL_TO1_0:
                    res = 0x00004df5;

            case    CHIP_MX6DQ_MAX:
            case    CHIP_MX6SDL_MAX:
            case    CHIP_MX6SL_MAX:
            case    CHIP_MX6SX_MAX:
            case    CHIP_MX7D_MAX:
            case    CHIP_MX6UL_MAX:
            case	CHIP_MX6ULL_MAX:
            case    CHIP_MX6SLL_MAX:
            case    CHIP_MAX:
                    break;
            }
            break;

    case    ROM_SYM_f_card_init:
            switch  (chip)  {
            case    CHIP_MX6DQ_TO1_0:
            case    CHIP_MX6DQ_TO1_1:
            case    CHIP_MX6DQ_TO1_2:
            case    CHIP_MX6DQ_TO1_3:
            case    CHIP_MX6DQ_TO1_4:
            case    CHIP_MX6DQ_TO1_5:
					res = 0x00003e7b;
                    break;
			case	CHIP_MX6DQ_TO2_0:
                    break;

            case    CHIP_MX6SDL_TO1_0:
            case    CHIP_MX6SDL_TO1_1:
            case    CHIP_MX6SDL_TO1_2:
            case    CHIP_MX6SDL_TO1_3:
                    break;

            case    CHIP_MX6SL_TO1_0:
            case    CHIP_MX6SL_TO1_1:
            case    CHIP_MX6SL_TO1_2:
                    break;

            case    CHIP_MX6SX_TO1_0:
                    res = 0x00002c17;
                    break;

            case    CHIP_MX6SX_TO1_1:
                    res = 0x00002c77;
                    break;

            case    CHIP_MX7D_TO1_0:
                    res = 0x0000345f;
                    break;

            case    CHIP_MX6UL_TO1_0:
                    res = 0x00002f95;
                    break;

			case    CHIP_MX6ULL_TO1_0:
					res = 0x00002fc5;
					break;

			case	CHIP_MX6SLL_TO1_0:
					res = 0x00002d35;
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
                    break;
            }
            break;

    case    ROM_SYM_v_hapi_src_reg:
            switch  (chip)  {
            case    CHIP_MX6DQ_TO1_0:
            case    CHIP_MX6DQ_TO1_1:
            case    CHIP_MX6DQ_TO1_2:
            case    CHIP_MX6DQ_TO1_3:
            case    CHIP_MX6DQ_TO1_4:
            case    CHIP_MX6DQ_TO1_5:
                    res = 0x009024b4;
                    break;
			case	CHIP_MX6DQ_TO2_0:
                    break;

            case    CHIP_MX6SDL_TO1_0:
            case    CHIP_MX6SDL_TO1_1:
            case    CHIP_MX6SDL_TO1_2:
            case    CHIP_MX6SDL_TO1_3:
                    break;

            case    CHIP_MX6SL_TO1_0:
            case    CHIP_MX6SL_TO1_1:
            case    CHIP_MX6SL_TO1_2:
                    break;

            case    CHIP_MX6SX_TO1_0:
                    res = 0x00900a28;
                    break;

            case    CHIP_MX6SX_TO1_1:
                    res = 0x00900a28;
                    break;

            case    CHIP_MX7D_TO1_0:
                    res = 0x009074b0;
                    break;

            case    CHIP_MX6UL_TO1_0:
                    res = 0x00900a2c;
                    break;

			case    CHIP_MX6ULL_TO1_0:
					res = 0x00900a2c;
					break;

            case    CHIP_MX6SLL_TO1_0:
                    printf("Get CHIP_MX6SLL_TO1_0!\n ");
                    res = 0x00900a30;
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
                    break;
            }
            break;

    case    ROM_SYM_v_port_esdhc:
            switch  (chip)  {
            case    CHIP_MX6DQ_TO1_0:
            case    CHIP_MX6DQ_TO1_1:
            case    CHIP_MX6DQ_TO1_2:
            case    CHIP_MX6DQ_TO1_3:
            case    CHIP_MX6DQ_TO1_4:
            case    CHIP_MX6DQ_TO1_5:
                    break;
			case	CHIP_MX6DQ_TO2_0:
                    break;

            case    CHIP_MX6SDL_TO1_0:
            case    CHIP_MX6SDL_TO1_1:
            case    CHIP_MX6SDL_TO1_2:
            case    CHIP_MX6SDL_TO1_3:
                    break;

            case    CHIP_MX6SL_TO1_0:
            case    CHIP_MX6SL_TO1_1:
            case    CHIP_MX6SL_TO1_2:
                    break;

            case    CHIP_MX6SX_TO1_0:
                    res = 0x00900a3c;
                    break;

            case    CHIP_MX6SX_TO1_1:
                    res = 0x00900a3c;
                    break;

            case    CHIP_MX7D_TO1_0:
                    res = 0x00907328;
                    break;

            case    CHIP_MX6UL_TO1_0:
                    res = 0x00900a44;
                    break;

			case    CHIP_MX6ULL_TO1_0:
					res = 0x00900a44;
					break;

			case    CHIP_MX6SLL_TO1_0:
					res = 0x00900a48;
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
                    break;
            }
            break;

    case    ROM_SYM_f_cfg_qspi_clock:
            switch  (chip)  {
            case    CHIP_MX6DQ_TO1_0:
            case    CHIP_MX6DQ_TO1_1:
            case    CHIP_MX6DQ_TO1_2:
            case    CHIP_MX6DQ_TO1_3:
            case    CHIP_MX6DQ_TO1_4:
            case    CHIP_MX6DQ_TO1_5:
                    break;
			case	CHIP_MX6DQ_TO2_0:
                    break;

            case    CHIP_MX6SDL_TO1_0:
            case    CHIP_MX6SDL_TO1_1:
            case    CHIP_MX6SDL_TO1_2:
            case    CHIP_MX6SDL_TO1_3:
                    break;

            case    CHIP_MX6SL_TO1_0:
            case    CHIP_MX6SL_TO1_1:
            case    CHIP_MX6SL_TO1_2:
                    break;

            case    CHIP_MX6SX_TO1_0:
                    res = 0x0000071f;
                    break;

            case    CHIP_MX6SX_TO1_1:
                    res = 0x000006e5;
                    break;

            case    CHIP_MX7D_TO1_0:
                    res = 0x000058b5;
                    break;

            case    CHIP_MX6UL_TO1_0:
                    res = 0x000006a3;
                    break;

			case    CHIP_MX6ULL_TO1_0:
					res = 0x000006a3;
					break;
					
			case	CHIP_MX6SLL_TO1_0:
					res = 0x000005f1;
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
                    break;
            }
            break;

    case    ROM_SYM_f_GPT_init:
            switch  (chip)  {
            case    CHIP_MX6DQ_TO1_0:
            case    CHIP_MX6DQ_TO1_1:
            case    CHIP_MX6DQ_TO1_2:
            case    CHIP_MX6DQ_TO1_3:
            case    CHIP_MX6DQ_TO1_4:
            case    CHIP_MX6DQ_TO1_5:
                    break;
			case	CHIP_MX6DQ_TO2_0:
                    break;

            case    CHIP_MX6SDL_TO1_0:
            case    CHIP_MX6SDL_TO1_1:
            case    CHIP_MX6SDL_TO1_2:
            case    CHIP_MX6SDL_TO1_3:
                    break;

            case    CHIP_MX6SL_TO1_0:
            case    CHIP_MX6SL_TO1_1:
            case    CHIP_MX6SL_TO1_2:
                    break;

            case    CHIP_MX6SX_TO1_0:
                    break;

            case    CHIP_MX6SX_TO1_1:
                    break;

            case    CHIP_MX7D_TO1_0:
                    res = 0x00000231;
                    break;

            case    CHIP_MX6UL_TO1_0:
                    break;

			case    CHIP_MX6ULL_TO1_0:
					break;

			case    CHIP_MX6SLL_TO1_0:
				    //res = 0x02098000;
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
                    break;
            }
            break;

    case    ROM_SYM_f_GPT_delay:
            switch  (chip)  {
            case    CHIP_MX6DQ_TO1_0:
            case    CHIP_MX6DQ_TO1_1:
            case    CHIP_MX6DQ_TO1_2:
            case    CHIP_MX6DQ_TO1_3:
            case    CHIP_MX6DQ_TO1_4:
            case    CHIP_MX6DQ_TO1_5:
					res = 0x0000577b;
					break;
            case    CHIP_MX6DQ_TO2_0:
                    res = 0x00005c3f;
                    break;

            case    CHIP_MX6SDL_TO1_0:
            case    CHIP_MX6SDL_TO1_1:
					res = 0x00005cd3;
                    break;
            case    CHIP_MX6SDL_TO1_2:
					res = 0x00005de7;
                    break;
            case    CHIP_MX6SDL_TO1_3:
					res = 0x00005e0b;
                    break;

            case    CHIP_MX6SL_TO1_0:
            case    CHIP_MX6SL_TO1_1:
            case    CHIP_MX6SL_TO1_2:
                    break;

            case    CHIP_MX6SX_TO1_0:
                    res = 0x00004d57;
                    break;

            case    CHIP_MX6SX_TO1_1:
                    res = 0x00004e47;
                    break;

            case    CHIP_MX7D_TO1_0:
                    res = 0x0000025d;
                    break;

            case    CHIP_MX6UL_TO1_0:
                    res = 0x00004ddb;
                    break;

			case    CHIP_MX6ULL_TO1_0:
					res = 0x00004e0b;
					break;

			case	CHIP_MX6SLL_TO1_0:
					res = 0x00004d8b;
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
                    break;
            }
            break;

    case    ROM_SYM_f_GPT_status:
            switch  (chip)  {
            case    CHIP_MX6DQ_TO1_0:
            case    CHIP_MX6DQ_TO1_1:
            case    CHIP_MX6DQ_TO1_2:
            case    CHIP_MX6DQ_TO1_3:
            case    CHIP_MX6DQ_TO1_4:
            case    CHIP_MX6DQ_TO1_5:
					res = 0x0000579b;
					break;
            case    CHIP_MX6DQ_TO2_0:
                    res = 0x00005c63;
                    break;

            case    CHIP_MX6SDL_TO1_0:
            case    CHIP_MX6SDL_TO1_1:
					res = 0x00005cf3;
                    break;
            case    CHIP_MX6SDL_TO1_2:
					res = 0x00005e07;
                    break;
            case    CHIP_MX6SDL_TO1_3:
					res = 0x00005e33;
                    break;

            case    CHIP_MX6SL_TO1_0:
            case    CHIP_MX6SL_TO1_1:
            case    CHIP_MX6SL_TO1_2:
                    break;

            case    CHIP_MX6SX_TO1_0:
                    res = 0x00004d7f;
                    break;

            case    CHIP_MX6SX_TO1_1:
                    res = 0x00004e6f;
                    break;

            case    CHIP_MX7D_TO1_0:
                    res = 0x0000026f;
                    break;

            case    CHIP_MX6UL_TO1_0:
                    res = 0x00004e25;
                    break;

			case    CHIP_MX6ULL_TO1_0:
					res = 0x00004e55;
					break;
					
            case    CHIP_MX6SLL_TO1_0:
					res = 0x00004dd5;
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
                    break;
            }
            break;
    case    ROM_SYM_f_GPT_reset_delay:
            switch  (chip)  {
            case    CHIP_MX6DQ_TO1_0:
            case    CHIP_MX6DQ_TO1_1:
            case    CHIP_MX6DQ_TO1_2:
            case    CHIP_MX6DQ_TO1_3:
            case    CHIP_MX6DQ_TO1_4:
            case    CHIP_MX6DQ_TO1_5:
					res = 0x0000574d;
					break;
            case    CHIP_MX6DQ_TO2_0:
                    res = 0x00005c0d;
                    break;

            case    CHIP_MX6SDL_TO1_0:
            case    CHIP_MX6SDL_TO1_1:
					res = 0x00005ca5;
                    break;
            case    CHIP_MX6SDL_TO1_2:
					res = 0x00005db9;
                    break;
            case    CHIP_MX6SDL_TO1_3:
					res = 0x00005dd5;
                    break;

            case    CHIP_MX6SL_TO1_0:
            case    CHIP_MX6SL_TO1_1:
            case    CHIP_MX6SL_TO1_2:
                    break;

            case    CHIP_MX6SX_TO1_0:
                    res = 0x00004d21;
                    break;

            case    CHIP_MX6SX_TO1_1:
                    res = 0x00004e11;
                    break;

            case    CHIP_MX7D_TO1_0:
                    res = 0x0000027f;
                    break;

            case    CHIP_MX6UL_TO1_0:
                    res = 0x00004da5;
                    break;

			case    CHIP_MX6ULL_TO1_0:
					res = 0x00004dd5;
					break;

			case    CHIP_MX6SLL_TO1_0:
					res = 0x00004d55;
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
                    break;
            }
            break;
    case    ROM_SYM_f_GPT_disable:
            switch  (chip)  {
            case    CHIP_MX6DQ_TO1_0:
            case    CHIP_MX6DQ_TO1_1:
            case    CHIP_MX6DQ_TO1_2:
            case    CHIP_MX6DQ_TO1_3:
            case    CHIP_MX6DQ_TO1_4:
            case    CHIP_MX6DQ_TO1_5:
					res = 0x000057a9;
					break;
            case    CHIP_MX6DQ_TO2_0:
                    res = 0x00005c71;
                    break;

            case    CHIP_MX6SDL_TO1_0:
            case    CHIP_MX6SDL_TO1_1:
					res = 0x00005d01;
                    break;
            case    CHIP_MX6SDL_TO1_2:
					res = 0x00005e15;
                    break;
            case    CHIP_MX6SDL_TO1_3:
					res = 0x00005e41;
                    break;

            case    CHIP_MX6SL_TO1_0:
            case    CHIP_MX6SL_TO1_1:
            case    CHIP_MX6SL_TO1_2:
                    break;

            case    CHIP_MX6SX_TO1_0:
                    res = 0x00004d8d;
                    break;

            case    CHIP_MX6SX_TO1_1:
                    res = 0x00004e7d;
                    break;

            case    CHIP_MX7D_TO1_0:
                    res = 0x0000027d;
                    break;

            case    CHIP_MX6UL_TO1_0:
                    res = 0x00004e33;
                    break;

			case    CHIP_MX6ULL_TO1_0:
					res = 0x00004e63;
					break;

            case    CHIP_MX6SLL_TO1_0:
					res = 0x00004de3;
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
                    break;
            }
            break;
    }

    return res;
}
