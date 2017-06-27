#include "io.h"
#include "platform.h"
#include "debug.h"
#include "serial_flash/imx_spi_nor.h"
#include "serial_flash/eeprom.h"
#include "shell/shell.h"
#include "timer/timer.h"
#include "rom_symbols.h"
#include "quadspi/qspi.h"

#define DEFAULT_LOAD_ADDRESS	(0x8fc400)

/*For pele*/
#define ROM_SBMR1_SERIAL_ROM		(0x30)
#define ROM_SBMR1_SPI_CS_MSK		(0x30000000)
#define ROM_SBMR1_SPI_CS_SHIFT		(28)
#define ROM_SBMR1_SPI_ADDR_16BIT 	(0x00000000)
#define ROM_SBMR1_SPI_ADDR_24BIT 	(0x08000000)
#define ROM_SBMR1_SERIAL_PORT_MSK 	(0x07000000)
#define ROM_SBMR1_SERIAL_PORT_SHIFT	(24)

#define ROM_SBMR1_QSPI_BOOT				(0x10)
#define ROM_SBMR1_QSPI_INSTANCE_SHIFT	(0x03)
#define ROM_SBMR1_QSPI1					(0x00)
#define ROM_SBMR1_QSPI2					(0x08)

#define QSPI1_BASE_ADDR		(0x60000000)
#define QSPI2_BASE_ADDR		(0x70000000)

// For pele debug only
#define IOMUXC_BASE_ADDR	(0x20e0000)
#define IOMUXC_SW_MUX_CTL_PAD_SD3_CLK (IOMUXC_BASE_ADDR + 0x250)
#define IOMUXC_SW_MUX_CTL_PAD_SD3_CMD	(IOMUXC_BASE_ADDR + 0x254 )
#define IOMUXC_SW_MUX_CTL_PAD_SD3_DATA2	(IOMUXC_BASE_ADDR + 0x260 )
#define IOMUXC_SW_MUX_CTL_PAD_SD3_DATA3	(IOMUXC_BASE_ADDR + 0x264 )

#define IOMUXC_SW_PAD_CTL_PAD_SD3_CLK (IOMUXC_BASE_ADDR + 0x598)
#define IOMUXC_SW_PAD_CTL_PAD_SD3_CMD (IOMUXC_BASE_ADDR + 0x59C)
#define IOMUXC_SW_PAD_CTL_PAD_SD3_DATA2 (IOMUXC_BASE_ADDR + 0x5a8)
#define IOMUXC_SW_PAD_CTL_PAD_SD3_DATA3 (IOMUXC_BASE_ADDR + 0x5ac)

#define IOMUXC_ECSPI4_IPP_CSPI_CLK_IN_SELECT_INPUT (IOMUXC_BASE_ADDR + 0x740)
#define IOMUXC_ECSPI4_IPP_CSPI_IND_MISO_SELECT_INPUT (IOMUXC_BASE_ADDR + 0x744)
#define IOMUXC_ECSPI4_IPP_CSPI_IND_MOSI_SELECT_INPUT (IOMUXC_BASE_ADDR + 0x748)
#define IOMUXC_ECSPI4_IPP_CSPI_IND_SS0_B_SELECT_INPUT (IOMUXC_BASE_ADDR + 0x74C)
// HYS, SRE, 40Ohm, Mid speed
#define SPI_PAD_SETTING 	((1 << 0) | (6 << 3) | (2 << 6) | (1 << 16))

typedef enum {
    SERIAL_BOOT_SPI1,
    SERIAL_BOOT_SPI2,
    SERIAL_BOOT_SPI3,
    SERIAL_BOOT_SPI4,
    SERIAL_BOOT_SPI5,
    SERIAL_BOOT_I2C1,
    SERIAL_BOOT_I2C2,
    SERIAL_BOOT_I2C3,
} serial_boot_dev_e;

extern chip_e chip;
extern uint32_t psp_get_spi_base(spi_port_e port);
extern uint32_t psp_get_i2c_base(i2c_port_e port);

static uint32_t spinor_init_flag = 0;

static void spinor_tool_help_list(void)
{
    printf("\n\nusage: spinor <command> [<args>]\n");
    printf("spinor command details:\n");
    printf("   init    Init SPI NOR\n");
    printf("           port    :   0: escpi1, 1: ecspi2, 2: escpi3, 3: ecspi4\n");
    printf("           cs      :   0: cs0\n");
    printf("   write   write data to flash\n");
    printf("           src     :   src data address\n");
    printf("           dst     :   offset on flash\n");
    printf("           len     :   binary size\n");
    printf("   read    read data from flash\n");
    printf("           src     :   offset on flash\n");
    printf("           dst     :   address on RAM\n");
    printf("           len     :   read size\n");
    printf("   es      erase the sepcific sector on the specific SPI NOR\n");
    printf("           addr    :   offset on flash\n");
    printf("           len     :   binary size\n");
    printf("   For example:\n");
    printf("       spinor init port 3 cs 0\n");
    printf("       spinor write src 0x910000 dst 0 len 0x4000\n");
    printf("       spinor es addr 0 len 1024\n");
}

static uint32_t flash_spinor_init(u32 port, u32 cs)//port and cs
{
    u32 base = 0;//define a variable base
    f_ecspi_init ecspi_dev_init = NULL;//define a 
    u32 id = 0;

    if (spinor_init_flag)
        return 0;

    base = psp_get_spi_base(port);//when port=0 base=0x02008000
    ecspi_dev_init = (f_ecspi_init)psp_rom_get_symble(ROM_SYM_f_ecspi_init, chip);//define a function pointer to point to the function in 6sll bootloader source code
//  ecspi_dev_init = 0x00004df5
    ecspi_dev_init(base);//represent a function, that is ecspi_init(base)
    printf("\necspi_init address = 0x%x\n", ecspi_dev_init);

    
    if (0 != spi_nor_init(base, cs)) {
        DEBUG_LOG("Failed to init SPI nor.\n");
        return 1;
    }

    spi_nor_get_id(&id);
    DEBUG_LOG("\n ID: 0x%x\n", id);

    spinor_init_flag = 1;

    return 0;
}

int do_spinor(unsigned int argc, char *argvs[])
{
    u32 port = 0, cs = 0;
    int i = 0;
    u32 src = 0, len = 0, dst = 0;

     if (argc < 2) {
        spinor_tool_help_list();
        return 0;
    }

    if (0 == strcmp(argvs[1], "help")) {
        spinor_tool_help_list();
    } else if (0 == strcmp(argvs[1], "init")) {
        if (argc < 6) {
            printf("\ntoo few arguments\n");
            return 0;
        }

        for (i = 2; i < argc; i++) {
            if (0 == strncmp(argvs[i], "port", sizeof("port"))) {
                if (0 != strtoul(argvs[i+1], &port)) {
                    printf("Invalid port.\n");
                    return -2;
                }

                i++;
            } else if (0 == strncmp(argvs[i], "cs", sizeof("cs"))) {
                if (0 != strtoul(argvs[i+1], &cs)) {
                    printf("Invalid chip select.\n");
                    return -3;
                }

                i++;
            } else {
                printf("\nInvalid arguments of spinor init command\n");
                return 0;
            }
        }

        return flash_spinor_init(port, cs);
    }
    else if (0 == strcmp(argvs[1], "es")) {
        if (argc < 6) {
            printf("\ntoo few arguments\n");
            return 0;
        }

        if (!spinor_init_flag) {
            printf("\nShould call init command to init spinor first!!\n");
            return 0;
        }

        for (i = 2; i < argc; i++) {
            if (0 == strncmp(argvs[i], "addr", sizeof("addr"))) {
                if (0 != strtoul(argvs[i+1], &dst)) {
                    printf("Invalid dst address.\n");
                    return -3;
                }

                i++;
            } else if (0 == strncmp(argvs[i], "len", sizeof("len"))) {
                if (0 != strtoul(argvs[i+1], &len)) {
                    printf("Invalid erase len.\n");
                    return -3;
                }

                i++;
            } else {
                printf("\nInvalid arguments of spinor es command\n");
                return 0;
            }
        }

        return spi_nor_erase(dst, len);
    }
    else if (0 == strcmp(argvs[1], "write")) {
        if (argc < 8) {
            printf("\ntoo few arguments\n");
            return 0;
        }

        if (!spinor_init_flag) {
            printf("\nShould call init command to init spinor first!!\n");
            return 0;
        }

        for (i = 2; i < argc; i++) {
            if (0 == strncmp(argvs[i], "dst", sizeof("dst"))) {
                if (0 != strtoul(argvs[i+1], &dst)) {
                    printf("Invalid dst address.\n");
                    return -3;
                }

                i++;
            } else if (0 == strncmp(argvs[i], "src", sizeof("src"))) {
                if (0 != strtoul(argvs[i+1], &src)) {
                    printf("Invalid src address.\n");
                    return -3;
                }

                i++;
            } else if (0 == strncmp(argvs[i], "len", sizeof("len"))) {
                if (0 != strtoul(argvs[i+1], &len)) {
                    printf("Invalid erase len.\n");
                    return -3;
                }

                i++;
            } else {
                printf("\nInvalid arguments of spinor write command\n");
                return 0;
            }
        }

        return spi_nor_write(dst, (u8 *)src, len);
    }
    else if (0 == strcmp(argvs[1], "read")) {
        if (argc < 8) {
            printf("\ntoo few arguments\n");
            return 0;
        }

        if (!spinor_init_flag) {
            printf("\nShould call init command to init spinor first!!\n");
            return 0;
        }

        for (i = 2; i < argc; i++) {
            if (0 == strncmp(argvs[i], "dst", sizeof("dst"))) {
                if (0 != strtoul(argvs[i+1], &dst)) {
                    printf("Invalid dst address.\n");
                    return -3;
                }

                i++;
            } else if (0 == strncmp(argvs[i], "src", sizeof("src"))) {
                if (0 != strtoul(argvs[i+1], &src)) {
                    printf("Invalid src address.\n");
                    return -3;
                }

                i++;
            } else if (0 == strncmp(argvs[i], "len", sizeof("len"))) {
                if (0 != strtoul(argvs[i+1], &len)) {
                    printf("Invalid erase len.\n");
                    return -3;
                }

                i++;
            } else {
                printf("\nInvalid arguments of spinor read command\n");
                return 0;
            }
        }

        return spi_nor_read((void *)src, (u8 *)dst, len);
    }
    else {
        printf("invalid argument\n");
        return 0;
    }

    return 0;
}

//SHELL_CMD(spinor, do_spinor, "spinor  SPI nor flash utility   Usage: spinor <commond> [<args>]")

#define EEPROM_BUF_SZ	128
u32 i2c_base = 0;

int do_i2c_eeprom(unsigned int argc, char *argvs[])
{
    int i = 0;
    u32 port = 0;
    u32 from, to, size, trans_bytes;
    u8 buf[EEPROM_BUF_SZ];

    memset(buf, 0, sizeof(buf));

    u32 *sbmr1_shadow = (u32 *) psp_rom_get_symble(ROM_SYM_v_SBMR_SHADOW, chip);
    f_u8_v f_setup_boot_sel = (f_u8_v) psp_rom_get_symble(ROM_SYM_f_SETUP_BOOT_SELECTION, chip);
    f_u8_v f_device_init = NULL;

    if (NULL == sbmr1_shadow || NULL == f_setup_boot_sel) {
        printf("Not supported on this chip yet.\n");
    }

    if (argc < 2) {
        printf("\nUsage: eeprom <command> [<args>]");
        return -1;
    }

    if (0 == strcmp(argvs[1], "init")) {
        for (i = 2; i < argc; i++) {
            if (0 == strncmp(argvs[i], "port=", 5)) {
                if (0 != strtoul(argvs[i] + 5, &port)) {
                    printf("\nInvalid port.\n");
                    return -2;
                }
            }
        }

        *sbmr1_shadow =
            ROM_SBMR1_SERIAL_ROM | (port + SERIAL_BOOT_I2C1) << ROM_SBMR1_SERIAL_PORT_SHIFT;
        f_setup_boot_sel();
        // Must be got after setup_boot_selection called.
        f_device_init = (f_u8_v) psp_rom_get_symble(ROM_SYM_f_device_init, chip);
        f_device_init();

        i2c_base = psp_get_i2c_base(port);

        /* read command: eeprom read from= size= */
    } else if (0 == strcmp(argvs[1], "read")) {
        if (0 == i2c_base) {
            printf("Error: not initialized yet.\n");
            return -4;
        }

        from = 0x0;
        size = 8192;
        for (i = 2; i < argc; i++) {
            if (0 == strncmp(argvs[i], "from=", 5)) {
                if (0 != strtoul(argvs[i] + 5, &from)) {
                    printf("\nInvalid from.\n");
                    return -5;
                }
            }
            if (0 == strncmp(argvs[i], "size=", 5)) {
                if (0 != strtoul(argvs[i] + 5, &size)) {
                    printf("\nInvalid size.\n");
                    return -6;
                }
            }
        }
        printf("\n");

        while (size > 0) {
            trans_bytes = size > EEPROM_BUF_SZ ? EEPROM_BUF_SZ : size;
            if (0 != at24cx_read(i2c_base, from, buf, trans_bytes)) {
                printf("Failed to read.\n");
                return -7;
            }
            for (i = 0; i < trans_bytes; i++) {
                printf("%02x ", buf[i]);
                if (i % 16 == 15)
                    printf("\n");
            }
            size -= trans_bytes;
            from += trans_bytes;
        }
        printf("bytes read complete!\n");

        /* write command: eeprom write from= size= to= */
    } else if (0 == strcmp(argvs[1], "write")) {
        if (0 == i2c_base) {
            printf("Error: not initialized yet.\n");
            return -4;
        }

        from = DEFAULT_LOAD_ADDRESS;
        size = 8192;            // ocram image length 8k
        to = 0x0;

        for (i = 2; i < argc; i++) {
            if (0 == strncmp(argvs[i], "from=", 5)) {
                if (0 != strtoul(argvs[i] + 5, &from)) {
                    printf("\nInvalid from.\n");
                    return -10;
                }
            }
            if (0 == strncmp(argvs[i], "size=", 5)) {
                if (0 != strtoul(argvs[i] + 5, &size)) {
                    printf("\nInvalid size.\n");
                    return -11;
                }
            }
            if (0 == strncmp(argvs[i], "to=", 3)) {
                if (0 != strtoul(argvs[i] + 3, &to)) {
                    printf("\nInvalid to.\n");
                    return -12;
                }
            }
        }
        printf("\n");

        while (size > 0) {
            trans_bytes = size > EEPROM_BUF_SZ ? EEPROM_BUF_SZ : size;
            if (0 != at24cx_write(i2c_base, to, (u8 *) from, trans_bytes)) {
                printf("Failed to write.\n");
                return -7;
            }

            size -= trans_bytes;
            from += trans_bytes;
            to += trans_bytes;
        }
        printf("bytes written complete\n");
    } else {
        printf("Invalid commond.\n");
    }

    return 0;
}

extern int quadspi_test_main(uint32_t index);

static void qspi_tool_help_list(void)
{
    printf("\n\nusage: qspinor <command> [<args>]\n");
    printf("qspinor command details:\n");
    printf("   init    init specific QSPI NOR\n");
    printf("           index   :   0: QSPI1, 1: QSPI2\n");
    printf("   program burn a binary image to specific QSPI NOR\n");
    printf("           index   :   0: QSPI1, 1: QSPI2\n");
    printf("           addr    :   source address\n");
    printf("           len     :   burning size\n");
    printf("           seek    :   skip bytes at the start of flash\n");
    printf("           skip    :   skip bytes at the start of source\n");
    printf("   ew      erase the whole of specific QSPI NOR\n");
    printf("           index   :   0: QSPI1, 1: QSPI2\n");
    printf("   es      erase the sepcific sector on the specific QSPI NOR\n");
    printf("           index   :   0: QSPI1, 1: QSPI2\n");
    printf("           addr    :   address\n");
    printf("   check   check data\n");
    printf("           index   :   0: QSPI1, 1: QSPI2\n");
    printf("           addr    :   example data address\n");
    printf("           len     :   example data size\n");
    printf("           init    :   QSPI host needs init or not, 0: not, 1: init\n");
    printf("           seek    :   skip bytes at the start of output\n");
    printf("           skip    :   skip bytes at the start of input\n");
    printf("   For example:\n");
    printf("       qspinor program index 1 addr 0x8FC000 len 0x4000 seek 0xc00 skip 0\n");
    printf("       qspinor ew index 1\n");
    printf("       qspinor es index 1 addr 0x70000000\n");
    printf("       qspinor check index 1 addr 0x8FC000 len 0x4000 init 0 seek 0xc00 skip 0\n");
}


int do_qspinor(unsigned int argc, char *argvs[])
{
    uint32_t i;
    uint32_t index=0, addr=0, len = 0, init = 0, skip = 0, seek = 0;

    if (argc < 2) {
        qspi_tool_help_list();
        return 0;
    }

    if (0 == strncmp(argvs[1], "help", sizeof"help")) {
        qspi_tool_help_list();
    } else if (0 == strncmp(argvs[1], "init", sizeof"init")) {
        if (argc < 4) {
            printf("\nUsage: qspinor <command> [<args>]\n");
            return 0;
        }

        for (i=2; i<argc; i++) {
            if (0 == strncmp(argvs[i], "index", sizeof"index")) {
                if (0 != strtoul(argvs[i+1], &index)) {
                    DEBUG_LOG("invalid index: %s\n", argvs[i+1]);
                    return -1;
                }
                i++;
            }
        }

        return quadspi_test_init(index);
    } else if (0 == strncmp(argvs[1], "program", sizeof"program")) {
        if (argc < 7) {
            printf("\nUsage: qspinor <command> [<args>]\n");
            return 0;
        }

        for (i=2; i<argc; i++) {
            if (0 == strncmp(argvs[i], "index", sizeof"index")) {
                if (0 != strtoul(argvs[i+1], &index)) {
                    DEBUG_LOG("invalid index: %s\n", argvs[i+1]);
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
                printf("\nUsage: qspinor <command> [<args>]\n");
                return 0;
            }
        }

        return quadspi_test_program(index, addr, len, seek, skip);
    } else if (0 == strncmp(argvs[1], "es", sizeof"es")) {
        if (argc < 3) {
            printf("\nUsage: qspinor <command> [<args>]\n");
            return 0;
        }

        for (i=2; i<argc; i++) {
            if (0 == strncmp(argvs[i], "index", sizeof"index")) {
                if (0 != strtoul(argvs[i+1], &index)) {
                    DEBUG_LOG("invalid index: %s\n", argvs[i+1]);
                    return -1;
                }
                i++;
            } else if (0 == strncmp(argvs[i], "addr", sizeof"addr")) {
                if (0 != strtoul(argvs[i+1], &addr)) {
                    DEBUG_LOG("invalid address: %s\n", argvs[i+1]);
                    return -1;
                }
                i++;
            } else {
                printf("\nUsage: qspinor <command> [<args>]\n");
                return 0;
            }
        }


        return quadspi_test_erase_sector(index, addr);
    } else if (0 == strncmp(argvs[1], "ew", sizeof"ew")) {
        if (argc < 3) {
            printf("\nUsage: qspinor <command> [<args>]\n");
            return 0;
        }

        for (i=2; i<argc; i++) {
            if (0 == strncmp(argvs[i], "index", sizeof"index")) {
                if (0 != strtoul(argvs[i+1], &index)) {
                    DEBUG_LOG("invalid index: %s\n", argvs[i+1]);
                    return -1;
                }
                i++;
            } else {
                printf("\nUsage: qspinor <command> [<args>]\n");
                return 0;
            }
        }


        return quadspi_test_erase_whole(index);
    } else if (0 == strncmp(argvs[1], "check", sizeof"check")) {
        if (argc < 7) {
            printf("\nUsage: qspinor <command> [<args>]\n");
            return 0;
        }

        for (i=2; i<argc; i++) {
            if (0 == strncmp(argvs[i], "index", sizeof"index")) {
                if (0 != strtoul(argvs[i+1], &index)) {
                    DEBUG_LOG("invalid index: %s\n", argvs[i+1]);
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
            } else if (0 == strncmp(argvs[i], "init", sizeof"init")) {
                if (0 != strtoul(argvs[i+1], &init)) {
                    DEBUG_LOG("invalid init flag: %s\n", argvs[i+1]);
                    return -1;
                }
                i++;
            } else if (0 == strncmp(argvs[i], "seek", sizeof"seek")) {
                if (0 != strtoul(argvs[i+1], &seek)) {
                    DEBUG_LOG("invalid seek number: %s\n", argvs[i+1]);
                    return -1;
                }
                i++;
            } else if (0 == strncmp(argvs[i], "skip", sizeof"skip")) {
                if (0 != strtoul(argvs[i+1], &skip)) {
                    DEBUG_LOG("invalid skip number: %s\n", argvs[i+1]);
                    return -1;
                }
                i++;
            } else {
                printf("\nUsage: qspinor <command> [<args>]\n");
                return 0;
            }
        }

        return quadspi_test_data_check(index, addr, len, init, seek, skip);
    } else {
        printf("\nUsage: qspinor <command> [<args>]\n");
        return 0;
    }

    return 0;
}

SHELL_CMD(eeprom, do_i2c_eeprom, "eeprom\t\teeprom utility\t\tUsage: eeprom <commond> [<args>]")
SHELL_CMD(qspinor, do_qspinor, "qspinor\t\tqspinor utility\t\tUsage: qspinor help")
SHELL_CMD(spinor, do_spinor, "spinor\t\tspinor utility\t\tUsage: spinor help")

