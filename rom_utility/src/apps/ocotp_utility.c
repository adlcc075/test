#include "io.h"
#include "ocotp/ocotp.h"
#include "shell/shell.h"

char ocotp_help[] = {
	"\nUsage: ocotp sense index\n"
	"         ocotp blow index value\n"
	"         ocotp blowbank bank word0 word1 word2 word3\n"
	"\n       For fuse below 0x800 in fuse map, the index = (fuse_addr - 0x400)/0x10\n"
	"\n       For fuse above 0x800, the index = (fuse_addr - 0x400)/0x10 - 0x10\n"
};

int do_ocotp(unsigned int argc, char *argvs[])
{
    unsigned int bank, row, val, index;
#ifdef MX7X
	unsigned int bank_val[4];
#endif

    if (argc < 3) {
		printf("%s\n", ocotp_help);
        return 0;
    }

    if (0 != strtoul(argvs[2], &index)) {
        return -1;
    }

    bank = index/8; row = index%8;

    if (0 == strcmp(argvs[1], "sense")) {
        printf("\n0x%08X\n", ocotp_sense_fuse(bank, row));
    } else if (0 == strcmp(argvs[1], "blow")) {
        if (argc != 4) {
            return -3;
        }
        if (0 != strtoul(argvs[3], &val)) {
            return -4;
        }

        ocotp_fuse_blow_row(bank, row, val);
        printf("\nSense it after blow: 0x%08X\n", ocotp_sense_fuse(bank, row));
    }
#ifdef MX7X
	else if (0 == strcmp(argvs[1], "blowbank")) {
        if (argc != 7) {
            return -3;
        }
        if ((0 != strtoul(argvs[3], &bank_val[0]))
			|| (0 != strtoul(argvs[4], &bank_val[1]))
			|| (0 != strtoul(argvs[5], &bank_val[2]))
			|| (0 != strtoul(argvs[6], &bank_val[3]))) {
            return -4;
        }

        ocotp_fuse_blow_bank(index, bank_val);
        printf("\nSense it after blow: 0x%08X, 0x%08X, 0x%08X, 0x%08X\n", 
				ocotp_sense_fuse(index/2, 0+index%2*4), ocotp_sense_fuse(index/2, 1+index%2*4), 
				ocotp_sense_fuse(index/2, 2+index%2*4), ocotp_sense_fuse(index/2, 3+index%2*4));
    }
#endif

    return 0;
}

SHELL_CMD(ocotp, do_ocotp, 
"ocotp\t\tSense/Blow fuse\t\tUsage: ocotp sense/blow index [value]")
