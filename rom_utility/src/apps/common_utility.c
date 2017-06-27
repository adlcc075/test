#include "io.h"
#include "shell/shell.h"

int do_mr(unsigned int argc, char *argvs[])
{
    int i;
    u32 addr, bytes = 4;
    u32 *p;
    if (argc < 2) {
        printf("\nUsage: mr <addr> [<bytes>]");
        return -1;
    }

    if (0 != strtoul(argvs[1], &addr)) {
        printf("\nInvalid addr\n");
        return -2;
    }

    if (argc > 2) {
        if (0 != strtoul(argvs[2], &bytes)) {
            printf("\n Invalid bytes");
            return -3;
        }
    }
    p = (u32*) ((addr >> 2) << 2);    //32 bits aligned
    printf("\n");
    for (i = 0; i < (bytes + 3) / sizeof(u32); i++, p++) {
        printf("%08x ", *p);
        if (i % 8 == 7)
            printf("\n");
    }
    printf("\n");

    return 0;
}

SHELL_CMD(mr, do_mr, 
"mr\t\tmemory read\t\tUsage: mr <addr> [<bytes>]")

int do_mw(unsigned int argc, char *argvs[])
{
    u32 addr, val;
    u32 *p;

    if (argc < 3) {
        printf("\nUsage: mr <addr> <val>");
        return -1;
    }

    if (0 != strtoul(argvs[1], &addr)) {
        printf("\nInvalid addr\n");
        return -2;
    }

    if (0 != strtoul(argvs[2], &val)) {
        printf("\nInvalid val\n");
        return -3;
    }

    p = (u32*) ((addr >> 2) << 2);    //32 bits aligned
    *p = val;

    printf("\n");

    return 0;
}

SHELL_CMD(mw, do_mw, 
"mw\t\tmemory write\t\tUsage: mw <addr> <val>")
