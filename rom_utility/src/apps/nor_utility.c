/*===================================================================================
                                        INCLUDE FILES
==================================================================================*/
#include "io.h"
#include "platform.h"
#include  "timer/timer.h"
#include  "shell/shell.h"
#include  "debug.h"
#include  "nor/nor.h"
#include "rom_symbols.h"

/*===================================================================================
                                        MACROS
==================================================================================*/
#define READ_BUFFER_SIZE 		(1024)

#define FIRMWARE_LENGTH_OFFSET	(0x24)

/*=================================================================================
                                 GLOBAL FUNCTION PROTOTYPES
=================================================================================*/
extern void start_pmccntr(void);
extern unsigned int read_stop_pmccntr(void);

/*=================================================================================
                                 GLOBAL varibles
=================================================================================*/
/* nor help strings */
char nor_help[] =
    "usage: nor <command> [<args>]\n"
    "nor command details:\n"
    "   info    print nor informations, include nor id, manufacture, block size etc\n"
    "   read    read some continuous words and dump them\n"
    "           address: start address of nor chip to read\n"
    "           length: bytes to read\n"
    "   erase   nor erase command, one block or full chip\n"
    "           block_addr: block address to be erase, -1 means full chip\n"
    "   write   nor write command, write one word into nor\n"
    "           address: nor chip address to write word\n"
    "           word: word to write into nor\n"
    "   burn    burn a binary image to nor flash\n"
    "           skip_KB: determin how many bytes to be skip, unit KB\n";

/* nor configuration information, including nor id, block_size, chip_size */
extern nor_cfg_t *g_nor_cfg;

/* cpu boot configuration related to NOR flash */
extern nor_bootcfg_t *g_nor_boot;
/*================================================================================
                          Hash Defines And Macros
=================================================================================*/
typedef int (*FUNC_ARGC0) (void);
typedef int (*FUNC_ARGC1) (int);
typedef int (*FUNC_ARGC2) (int, int);
typedef int (*FUNC_ARGC3) (int, int, int);
typedef int (*FUNC_ARGC4) (int, int, int, int);
typedef int (*FUNC_ARGC5) (int, int, int, int, int);
typedef int (*FUNC_ARGC6) (int, int, int, int, int, int);
typedef int (*FUNC_ARGC7) (int, int, int, int, int, int, int);
typedef int (*FUNC_ARGC8) (int, int, int, int, int, int, int, int);

/* nor cmd struct */
typedef struct nor_cmd {
    char name[10];
    void *pfunc;
    int argc;
    char help[150];
} nor_cmd_t;

/*==========================================================================
                                     Global FUNCTIONS
==========================================================================*/
/** 
  * cmd_nor_info - print nor information,
                   include ids, block size, chip size
  *
  * return 0 if succeed, otherwise return -1
  */
int cmd_nor_info(void)
{
	nor_cfg_dump();

	return 0;
}

/** 
  * cmd_nor_erase - erase nor flash,
  * @block_addr: block address to erase, < 0 means erase full chip
  *
  * return 0 if succeed, otherwise return -1
  */
int cmd_nor_erase(int block_addr)
{
	nor_cmdset_t *cmd = g_nor_cfg->cmdset;

	if(block_addr < 0) {
		printf("begin erase full chip!\n");
		if((cmd->erase_chip == NULL) || !(*cmd->erase_chip)()) {
			printf("erase full chip failed!\n");
			return -1;
		}
		printf("erase full chip complete!\n");
	} else {
		printf("begin erase block %d\n", block_addr);
		if(!(*cmd->erase_block)(block_addr)) {
			printf("erase block %d failed!\n", block_addr);
			return -1;
		}
		printf("erase block %d complete!\n", block_addr);
	}

	return 0;
}

/** 
  * cmd_nor_read - read nor flash, and dump them in word format
  * @addr: nor chip start address to read, not cpu address
  * @len: number of bytes to read
  *
  * return 0 if succeed, otherwise return -1
  */
int cmd_nor_read(int addr, int len)
{
	U16 buf[READ_BUFFER_SIZE];
	int i;
	int step_len;
	nor_cmdset_t *cmd = g_nor_cfg->cmdset;

	/* test addr whether word align */
	if(addr % 2 != 0) {
		printf("data bus is 16bit, address must be even!\n");
		printf("read begin from %d\n", addr -1);
		addr -= 1;
	}

	/* read and dump words */
	while(len > 0) {
		step_len = ((len > READ_BUFFER_SIZE) ? READ_BUFFER_SIZE : len);

		/* read words from nor */
		(*cmd->read_words)(addr, step_len, buf);

		/* dump datas, word format */
		printf("nor data 0x%x ~ 0x%x:\n", addr, addr + step_len);
		for(i = 0; i < (step_len + 1) / 2; i++) {
			if((i != 0) && (i % 8) == 0)
				printf("\n");
			else if((i != 0) && (i % 2 == 0))
				printf("    ");

			printf("%4x ", buf[i]);
		}
		printf("\n");

		/* address and len change */
		addr += step_len;
		len -= step_len;
	}

	return 0;
}

/** 
  * cmd_nor_write - write one word into nor flash
  * @addr: nor chip address to write, not cpu address
  * @word: word to write into nor
  *
  * return 0 if succeed, otherwise return -1
  */
int cmd_nor_write(int addr, int word)
{
	nor_cmdset_t *cmd = g_nor_cfg->cmdset;

	/* test addr whether word align */
	if(addr % 2 != 0) {
		printf("data bus is 16bit, address must be even!\n");
		return -1;
	}

	/* write word into nor flash */
	if(!(*cmd->write_words)(addr, (U16 *)(&word), 1)) {
		printf("nor write word at address 0x%x failed!\n", addr);
		return -1;
	}

	printf("nor write word at address 0x%x complete!\n", addr);
	return 0;
}

/** 
  * cmd_nor_burn - burn firmware into nor
  * @skip_KB: number of bytes in KB unit to skip nor flash
  *
  * return 0 if succeed, otherwise return -1
  */
int cmd_nor_burn(int skip_KB)
{
	U16 *firmware_addr;
	U32 firmware_len;
	U32 addr_start;
	U32 i;
	nor_cmdset_t *cmd = g_nor_cfg->cmdset;

	/* get firmware real length from image that is load into ddr by TRACE32 */ 
	firmware_len = *((U32 *) (g_nor_boot->firmware_load_addr + 
		skip_KB * 1024 + FIRMWARE_LENGTH_OFFSET));
	firmware_addr = (U16 *)(g_nor_boot->firmware_load_addr);

	/* erase blocks to burn firmware first */
	for(i = 0; i < ((firmware_len - 1) / g_nor_cfg->block_size + 1); i++) {
		if(!(*cmd->erase_block)(i)) {
			printf("erase block %d failed!\n", i);
			return -1;
		}
	}
	
	/* write words into nor flash */
	addr_start = g_nor_boot->ivt_offset - skip_KB * 1024;
	if(!(*cmd->write_words)(addr_start, firmware_addr, firmware_len)) {
		printf("nor write words failed, burn failed!\n");
		return -1;
	}

	printf("burn firmware succeed!\n");
	return 0;
}

/* nor command array */
static nor_cmd_t cmd[] = {
    {"info", 	cmd_nor_info, 	0, 	"nor info"},
    {"erase", 	cmd_nor_erase, 	1, 	"nor erase block_addr"},
    {"read", 	cmd_nor_read, 	2, 	"nor read address length"},
    {"write", 	cmd_nor_write, 	2, 	"nor write address word"},
    {"burn", 	cmd_nor_burn, 	1, 	"nor burn skip_KB"},
};


/** 
  * do_nor - nor comand parase
  * @argc: argument numbers
  * @argvs: argument strings array
  *
  * return 0 if succeed, return -1 otherwise
  */
int do_nor(unsigned int argc, char *argvs[])
{
    int i, j;
    void *pfunc;
    int arg[10];
	extern chip_e chip;		// determin which chip we are using

    if ((argc < 2) || (0 == strncmp(argvs[1], "help", sizeof"help"))) {
        printf("%s\n", nor_help);
        return 0;
    }

	/* shell task talk init */
    start_pmccntr();
    read_stop_pmccntr();
    printf("\n");

	/* nor init first */
	if (!nor_boot_chip_cfg(chip)) {
		return -1;
	}

    /* search cmd from cmd array if the input name matched */
    for (i = 0; i < ARRAY_SIZE(cmd); i++) {
        /* compare to judge whether name matched */
        if (0 == strncmp(argvs[1], cmd[i].name, sizeof(argvs[1]))) {
            /* check whether argc matched */
            if (argc != cmd[i].argc + 2) {
                printf("input argument number don't match \n%s\n", cmd[i].help);
                return -1;
            } else {
                pfunc = cmd[i].pfunc;

                /* check input argument, if not integer, return -1 */
                for (j = 0; j < cmd[i].argc; j++) {
                    if (0 != strtol(argvs[j + 2], &arg[j])) {
                        printf("input argument error \n%s\n", cmd[i].help);
                        return -1;
                    }
                }

                /* call function as the cmd argument */
                switch (cmd[i].argc) {
                case 0:
                    return (*(FUNC_ARGC0) pfunc) ();
                case 1:
                    return (*(FUNC_ARGC1) pfunc) (arg[0]);
                case 2:
                    return (*(FUNC_ARGC2) pfunc) (arg[0], arg[1]);
                case 3:
                    return (*(FUNC_ARGC3) pfunc) (arg[0], arg[1], arg[2]);
                case 4:
                    return (*(FUNC_ARGC4) pfunc) (arg[0], arg[1], arg[2], arg[3]);
                case 5:
                    return (*(FUNC_ARGC5) pfunc) (arg[0], arg[1], arg[2], arg[3], arg[4]);
                case 6:
                    return (*(FUNC_ARGC6) pfunc) (arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]);
                case 7:
                    return (*(FUNC_ARGC7) pfunc) (arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], arg[6]);
                case 8:
                    return (*(FUNC_ARGC8) pfunc) (arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], arg[6], arg[7]);
                default:
                    printf("input argument number too many\n%s\n", cmd[i].help);
                    return -1;
                }
            }
        }
    }

    printf("command not found\n%s\n", nor_help);
    return -1;
}

SHELL_CMD(nor, do_nor, "nor\t\tnor utility\t\tUsage: nor help")
