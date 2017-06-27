#ifndef __SHELL_H__
#define __SHELL_H__

typedef struct {
    char *name;
    int (*func) (unsigned int argc, char *argvs[]);
    char *short_usage;
} shell_cmd_t, *shell_cmd_p;

#define SHELL_CMD(name, func, short_usage) __attribute__ ((section(".shell_cmd")))	\
	shell_cmd_t shell_cmd_##name = {	\
			#name, 	\
			func,	\
			short_usage \
			};

#define PROMPT_STRING	"\nROM_Utility->"

int shell_task_loop(unsigned int para);

	
#endif
