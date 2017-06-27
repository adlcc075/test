
#include "platform.h"
#include "debug.h"
#include "rom_symbols.h"
#include "boot_cfg.h"
#include "usdhc/usdhc.h"
#include "timer/timer.h"
#include "shell/shell.h"

extern uint32_t get_usdhc_base(usdhc_port_e port);
extern void start_pmccntr(void);
extern unsigned int read_stop_pmccntr(void);

extern chip_e chip;

static uint32_t card_init_flag_g = 0;

//#define BIN_LOAD_ADDR 0x27800000
//#define BIN_LOAD_ADDR 0x8fc000

enum {
    HOST,
    OCR,
    CID,
    CSD,
    ECSD,
    RCA,
    DSR,
};

#define SDR 1
#define DDR 2

static void mmc_tool_help_list(void)
{
    printf("\n\nusage: mmc <command> [<args>]\n");
    printf("mmc command details:\n");
    printf("   burn    burn a binary image to the specific partition on mmc/emmc\n");
    printf("           port    :   usdhc port, 1~4\n");
    printf("           part    :   which area will burn image to, 0: user area, 1: boot partition 1, 2:boot partition 2\n");
    printf("           addr    :   binary address\n");
    printf("           len     :   binary size\n");
    printf("           seek    :   skip bytes at the start of output\n");
    printf("           skip    :   skip bytes at the start of input\n");
    printf("   dump    dump the specfic register, include host and slave\n");
    printf("           port    :   usdhc port, 1~4\n");
    printf("           host    :   dump usdhc host registers\n");
    printf("           ocr     :   dump OCR register of MMC\n");
    printf("           cid     :   dump CID register of MMC\n");
    printf("           csd     :   dump CSD register of MMC\n");
    printf("           ecsd    :   dump extended CSD register of MMC\n");
    printf("           rca     :   dump RCA register of MMC\n");
    printf("           dsr     :   dump DSR register of MMC\n");
    printf("   btcfg   boot config\n");
    printf("           port    :   usdhc port, 1~4\n");
    printf("           part    :   boot area option, 0: user area, 1: boot partition 1, 2: boot partition 2\n");
    printf("           ack     :   acknowledge options, 0: disable acknowledge, 1: enable acknowledge\n");
    printf("           width   :   boot bus width, 1: 1 bit, 4: 4 bits, 8: 8 bits\n");
    printf("           bt      :   sdr or ddr, 1: sdr, 2: ddr\n");
    printf("   For example:\n");
    printf("       mmc burn port 1 part 1 addr 0x900000 len 0x6000 seek 0x400 skip 0x400  "
"; burn a binary image to mmc boot partition 1, image size is 0x6000, buffer on 0x900000, the mmc card connects with usdhc1\n");
    printf("       mmc dump port 1 ecsd   ; dump extended CSD register of the mmc card which connects with usdhc1\n");
    printf("       mmc btcfg port 1 part 0 ack 1 width 1 bt 1  ; boot from user area, ackonwlege is enabled, bus width is 1 bit, sdr\n");
}

static uint32_t card_mmc_init(uint32_t port)
{
    f_u8_v card_dev_init = NULL;
    uint32_t instance = 0;
    uint32_t *port_esdhc_addr = NULL;

    if (card_init_flag_g)
        return 0;

    if (0 != boot_cfg_override(MMC_BOOT, port, chip, (u8 *)0)) {
        DEBUG_LOG("override boot cfg failure\n");
        return 1;
    }

    port_esdhc_addr = (uint32_t *)psp_rom_get_symble(ROM_SYM_v_port_esdhc, chip);
    if (!port_esdhc_addr) {
        DEBUG_LOG("Failed to get address of the variable port_esdhc\n");
        return 1;
    }
	printf("port_esdhc_addr=0x%x\n",port_esdhc_addr);

    *port_esdhc_addr = 0x000000ff;

    card_dev_init = (f_u8_v)psp_rom_get_symble(ROM_SYM_f_card_init, chip);
    if (!card_dev_init) {
        DEBUG_LOG("Failed to get address of card_init() func\n");
        return 1;
    }
	printf("card_dev_init=0x%x\n",card_dev_init);

    if (TRUE != card_dev_init()) {
        DEBUG_LOG("Call card_init() failure\n");
        return 1;
    }

    /* get base address accroding usdhc port number */
    instance = get_usdhc_base((usdhc_port_e)(port - 1));
    if (!instance) {
        DEBUG_LOG("get usdhc base address failure\n");
        return 1;
    }
	printf("instance=%d\n",instance);

    mmc_get_version_simple(instance);

    card_init_flag_g = 1;

    return 0;
}

static int mmc_burn(uint32_t port, uint32_t part, uint32_t addr, uint32_t len, uint32_t seek, uint32_t skip)
{
    uint32_t instance;

    /* init mmc card */
    if (0 != card_mmc_init(port)) {
        DEBUG_LOG("init mmc card failure\n");
        return -1;
    }

    /* map base address of usdhc according to port number */
    instance = get_usdhc_base((usdhc_port_e)(port - 1));
    if (!instance) {
        DEBUG_LOG("get usdhc base address failure\n");
        return -1;
    }

    /* configure mmc area access */
    if (FAIL == mmc_set_bp_access(instance, (emmc_part_e)part)){
        DEBUG_LOG("configure card access failure\n");
        return -1;
    }

    /* push data to mmc card */
    if (FAIL ==
        card_data_write(instance,
                        (int *)(addr + skip),
                        (len - skip ),
                        seek)) {
        DEBUG_LOG("push data to card failure\n");
        return -1;
    }

    return 0;
}

static int mmc_dump(uint32_t port, uint32_t type)
{
    unsigned int instance;

    /* init mmc card */
    if (0 != card_mmc_init(port)) {
        DEBUG_LOG("init mmc card failure\n");
        return -1;
    }

    /* map base address of usdhc according to port number */
    instance = get_usdhc_base((usdhc_port_e)(port - 1));
    if (!instance) {
        DEBUG_LOG("get usdhc base address failure\n");
        return -1;
    }

    switch  (type)  {

    case    HOST:
            host_registers_dump(instance);
            break;

    case    OCR:
            mmc_dump_ocr((usdhc_port_e)(port - 1));
            break;

    case    CID:
            card_dump_cid((usdhc_port_e)(port - 1));
            break;

    case    CSD:
            mmc_dump_csd((usdhc_port_e)(port - 1));
            break;

    case    ECSD:
            mmc_dump_ecsd(instance);
            break;

    case    RCA:
            break;

    case    DSR:
            break;
    }

    return 0;
}

static int mmc_boot_config(uint32_t port, uint32_t part, bool ack, uint32_t width, uint32_t bt)
{
    uint32_t instance;
    emmc_bus_width_e bus_cfg;

    /* init mmc card */
    if (0 != card_mmc_init(port)) {
        DEBUG_LOG("init mmc card failure\n");
        return -1;
    }

    /* get base address accroding usdhc port number */
    instance = get_usdhc_base((usdhc_port_e)(port - 1));
    if (!instance) {
        DEBUG_LOG("get usdhc base address failure\n");
        return -1;
    }

    /* set boot area of mmc card */
    if (FAIL == mmc_set_boot_config(instance, (emmc_part_e)part, ack)) {
        DEBUG_LOG("select boot area failure\n");
        return -1;
    }

    if (SDR == bt) {
        switch  (width) {

        case    1:
                bus_cfg = EMMC_BOOT_SDR1;
                break;

        case    4:
                bus_cfg = EMMC_BOOT_SDR4;
                break;

        case    8:
                bus_cfg = EMMC_BOOT_SDR8;
                break;

        default:
                DEBUG_LOG("invalid bus width(%d) for SDR\n", width);
                return -1;
        }
    } else if (DDR == bt) {
        if (4 == width)
            bus_cfg = EMMC_BOOT_DDR4;
        else if (8 == width)
            bus_cfg = EMMC_BOOT_DDR8;
        else {
            DEBUG_LOG("invalid bus width(%d) for DDR\n", width);
            return -1;
        }
    }

    if (FAIL == mmc_set_boot_bus_width(instance, bus_cfg)) {
        DEBUG_LOG("set boot bus width failure\n");
        return -1;
    }

    return 0;
}

int do_mmc(unsigned int argc, char *argvs[])
{
#define INVALID_VALUE 0xffffffff
    uint32_t port = INVALID_VALUE, partition = INVALID_VALUE;
    uint32_t len = INVALID_VALUE, type = INVALID_VALUE, ack = 0, addr = 0;
    uint32_t skip = 0, seek = 0, width = INVALID_VALUE, bt = INVALID_VALUE;
    uint32_t i;
    //uint32_t cnt = 0;

    if (argc < 2) {
        mmc_tool_help_list();
        return 0;
    }

    start_pmccntr();

    GPT_init();

    //cnt = read_stop_pmccntr();
    if (0 == strncmp(argvs[1], "help", sizeof"help")) {
        mmc_tool_help_list();
        return 0;
    } else if (0 == strncmp(argvs[1], "burn", sizeof"burn")) {
        if (argc < 12) {
            printf("\nUsage: mmc <command> [<args>]\n");
            return 0;
        }

        for (i=2; i<argc; i++) {
            if (0 == strncmp(argvs[i], "port", sizeof"port")) {
                if (0 != strtoul(argvs[i+1], &port)) {
                    DEBUG_LOG("invalid port number: %s\n", argvs[i+1]);
                    return -1;
                }
                i++;
            } else if (0 == strncmp(argvs[i], "part", sizeof"part")) {
                if (0 != strtoul(argvs[i+1], &partition)) {
                    DEBUG_LOG("invalid partition number: %s\n", argvs[i+1]);
                    return -1;
                }
                i++;
            } else if (0 == strncmp(argvs[i], "addr", sizeof"addr")) {
                if (0 != strtoul(argvs[i+1], &addr)) {
                    DEBUG_LOG("invalid address: %s\n", argvs[i+1]);
                    return -1;
                }
                i++;
            } else if (0 == strncmp(argvs[i], "len", sizeof"len")) {
                if (0 != strtoul(argvs[i+1], &len)) {
                    DEBUG_LOG("invalid length number: %s\n", argvs[i+1]);
                    return -1;
                }
                i++;
            } else if (0 == strncmp(argvs[i], "seek", sizeof"seek")) {
                if (0 != strtoul(argvs[i+1], &seek)) {
                    DEBUG_LOG("invalid seek number: %s\n", argvs[i+1]);
                    return -1;
                }
                i++;
            }  else if (0 == strncmp(argvs[i], "skip", sizeof"skip")) {
                if (0 != strtoul(argvs[i+1], &skip)) {
                    DEBUG_LOG("invalid skip number: %s\n", argvs[i+1]);
                    return -1;
                }
                i++;
            } else {
                printf("\nUsage: mmc <command> [<args>]\n");
                return 0;
            }
        }

        if ((INVALID_VALUE == port) || (INVALID_VALUE == partition) ||
            (INVALID_VALUE == len)) {
            printf("\nUsage: mmc <command> [<args>]\n");
            return 0;
        }

        return mmc_burn(port, partition, addr, len, seek, skip);
    } else if (0 == strncmp(argvs[1], "dump", sizeof"dump")) {
        if (5 != argc) {
            printf("\nUsage: mmc <command> [<args>]\n");
            return 0;
        }

        for (i=2; i<argc; i++) {
            if (0 == strncmp(argvs[i], "port", sizeof"port")) {
                if (0 != strtoul(argvs[i+1], &port)) {
                    DEBUG_LOG("invalid port number: %s\n", argvs[i+1]);
                    return -1;
                }
                i++;
            } else if (0 == strncmp(argvs[i], "host", sizeof"host")) {
                type = HOST;
            } else if (0 == strncmp(argvs[i], "ocr", sizeof"ocr")) {
                type = OCR;
            } else if (0 == strncmp(argvs[i], "cid", sizeof"cid")) {
                type = CID;
            } else if (0 == strncmp(argvs[i], "csd", sizeof"csd")) {
                type = CSD;
            } else if (0 == strncmp(argvs[i], "ecsd", sizeof"ecsd")) {
                type = ECSD;
            } else if (0 == strncmp(argvs[i], "rca", sizeof"rca")) {
                type = RCA;
            } else if (0 == strncmp(argvs[i], "dsr", sizeof"dsr")) {
                type = DSR;
            } else {
                printf("\nUsage: mmc <command> [<args>]\n");
                return 0;
            }
        }

        if ((INVALID_VALUE == port) || (INVALID_VALUE == type)) {
            printf("\nUsage: mmc <command> [<args>]\n");
            return 0;
        }

        return mmc_dump(port, type);
    } else if (0 == strncmp(argvs[1], "btcfg", sizeof"btcfg")){
        if (12 != argc) {
            printf("\nUsage: mmc <command> [<args>]\n");
            return 0;
        }

        for (i=2; i<argc; i++) {
            if (0 == strncmp(argvs[i], "port", sizeof"port")) {
                if (0 != strtoul(argvs[i+1], &port)) {
                    DEBUG_LOG("invalid port number: %s\n", argvs[i+1]);
                    return -1;
                }
                i++;
            } else if (0 == strncmp(argvs[i], "part", sizeof"part")) {
                if (0 != strtoul(argvs[i+1], &partition)) {
                    DEBUG_LOG("invalid partition number: %s\n", argvs[i+1]);
                    return -1;
                }
                i++;
            } else if (0 == strncmp(argvs[i], "ack", sizeof"ack")) {
                if (0 != strtoul(argvs[i+1], &ack)) {
                    DEBUG_LOG("invalid length number: %s\n", argvs[i+1]);
                    return -1;
                }
                i++;
            } else if (0 == strncmp(argvs[i], "width", sizeof"width")) {
                if (0 != strtoul(argvs[i+1], &width)) {
                    DEBUG_LOG("invalid partition number: %s\n", argvs[i+1]);
                    return -1;
                }
                i++;
            } else if (0 == strncmp(argvs[i], "bt", sizeof"bt")) {
                if (0 != strtoul(argvs[i+1], &bt)) {
                    DEBUG_LOG("invalid partition number: %s\n", argvs[i+1]);
                    return -1;
                }
                i++;
            } else {
                printf("\nUsage: mmc <command> [<args>]\n");
                return 0;
            }
        }

        if ((INVALID_VALUE == port) || (INVALID_VALUE == partition) ||
            (INVALID_VALUE == width) || (INVALID_VALUE == bt)) {
            printf("\nUsage: mmc <command> [<args>]\n");
            return 0;
        }

        return mmc_boot_config(port, partition, (bool)ack, width, bt);
    } else {
        printf("\nUsage: mmc <command> [<args>]\n");
        return 0;
    }
}

SHELL_CMD(mmc, do_mmc, "mmc\t\tmmc utility\t\tUsage: mmc help")

