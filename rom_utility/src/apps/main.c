#include "platform.h"
//#include "log_parser.h"
#include "nand_parser.h"
#include "hab_parser.h"
#include "shell/shell.h"

#define ROM_LOG_PARSER_ENABLE		1
#define ROM_TOOL_DEBUG 1
#define PAESER_VERSION_STR "0.0.1beta"

extern int get_chip_type(void);
extern int platform_init(chip_e chip);

chip_e chip;

int main(void)
{
    chip = get_chip_type();
	printf("chip = %d\n", chip);
    if (chip == CHIP_MAX) {
        printf("Chip or Tape-out not supported.\n");
        while (1) ;
    }
    platform_init(chip);

    printf("\n\n*******************************************\n");
    printf("    ROM utility suite, for ROM validation\n");
    printf("*********************************************\n\n");

    shell_task_loop(NULL); 

    return 0;
}
