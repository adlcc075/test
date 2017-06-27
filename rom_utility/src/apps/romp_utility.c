#include "io.h"
#include "platform.h"
#include "shell/shell.h"

#define romcp_base_address 	0x21AC000
#define ROMC_ROMPATCHD(n) ((unsigned int)romcp_base_address + 0xd4 + ((n)*4))
#define ROMC_ROMPATCHCNTL ((unsigned int)romcp_base_address + 0xf4)
#define ROMC_ROMPATCHENH  ((unsigned int)romcp_base_address + 0xf8)
#define ROMC_ROMPATCHENL  ((unsigned int)romcp_base_address + 0xfc)
#define ROMC_ROMPATCHA(n) ((unsigned int)romcp_base_address + 0x100 + ((n)*4))
#define ROMC_ROMPATCHSR   ((unsigned int)romcp_base_address + 0x208)

int do_rom_patch(unsigned int argc, char *argvs[])
{
    u32 ch = 0;
    // data fix
    writel(0x900d0bad, ROMC_ROMPATCHD(7 - ch));
    writel(0x0, ROMC_ROMPATCHA(ch));
    writel(1 << ch, ROMC_ROMPATCHCNTL);
    writel(1 << ch, ROMC_ROMPATCHENL);

    printf("\n 0x%x\n", *(u32 *) 0);

    return 0;
}

SHELL_CMD(rom_patch, do_rom_patch,
          "rom_patch 	rom patch utility	Usage: rom_patch <commond> [<args>]")
