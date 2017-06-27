#include "io.h"
#include "shell.h"

#define SHELL_HISTORY_SUPPORT	1

#define TERM_INOUT_BUFFER_SZ	256
#define MAX_ARGV_NUM		32
#define HISTORY_CMD_MUX		4

extern shell_cmd_t __shell_cmds_beginning[], __shell_cmds_end[];

char cmd_line_buffer[TERM_INOUT_BUFFER_SZ];
char *argvs[MAX_ARGV_NUM];
#if SHELL_HISTORY_SUPPORT
char history_buffer[4][TERM_INOUT_BUFFER_SZ];
int history_buf_idx = 0;
int history_buf_wlk_idx = 0;
int cmds_total = 0;
#endif
int cmd_line_process(char *cmd_line, unsigned int *argc, char *argvs[])
{
    char *p = cmd_line;
    char *start = NULL;
    unsigned int cnt = 0;
    unsigned int beginning_found = FALSE;

    while (0 != *p) {
        if (!beginning_found) {
            if (*p == ' ' || *p == '\t') {
                p++;
                continue;
            } else {
                start = p;
                argvs[cnt++] = start;
                beginning_found = TRUE;
            }
        }

        if (' ' == *p || '\t' == *p) {
            *p = 0;
            beginning_found = FALSE;
        }

        p++;
    }

    *argc = cnt;

    return 0;
}

int shell_task_loop(unsigned int para)
{
    char *p = (char *)(&cmd_line_buffer[0]);
    char ch = 0, c = 0;
    unsigned int argc = 0;
    shell_cmd_p cmd_start = (shell_cmd_p) __shell_cmds_beginning;
    shell_cmd_p cmd_end = (shell_cmd_p) __shell_cmds_end;
    shell_cmd_p cmd;
    int i = 0;

    para = para;

    memset(cmd_line_buffer, 0, sizeof(cmd_line_buffer));
    memset(argvs, 0, sizeof(argvs));
    memset(history_buffer, 0, sizeof(history_buffer));

    printf(PROMPT_STRING);

    while (1) {
        if (0 == get_char((unsigned char *)&ch)) {
            *p = ch;
            if (ch != '\r') {
                if ('\b' == ch) {
                    if (p > (char *)(&cmd_line_buffer[0])) {
                        /* a trick for the backspace, to remove one char back */
                        printf("%c", '\b');
                        printf(" ");
                        printf("%c", '\b');
                        p--;
                    }
                    continue;
                } else {
                    if (!((ch == '<') || (ch == '>'))) {    // Skip the case '>' or '<' 
                        printf("%c", ch);   // echo it
                    }
                }
            }

            /* Walk the history commands if "<" or ">"" pressed firstly */
            if (('<' == ch) || ('>' == ch)) {
                if (cmds_total > 0) {
                    if ('<' == ch) {
                        history_buf_wlk_idx--;
                        if (history_buf_wlk_idx < 0) {
                            if (cmds_total >= HISTORY_CMD_MUX)
                                history_buf_wlk_idx = HISTORY_CMD_MUX - 1;
                            else
                                history_buf_wlk_idx = 0;
                        }
                    } else {
                        history_buf_wlk_idx++;
                        if (history_buf_wlk_idx >= HISTORY_CMD_MUX) {
                            if (cmds_total >= HISTORY_CMD_MUX)
                                history_buf_wlk_idx = 0;
                            else
                                history_buf_wlk_idx = HISTORY_CMD_MUX - 1;
                        }
                    }

                    c = history_buffer[history_buf_wlk_idx][0];
                    /* at least it is command-like stuff */
                    if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'))) {
                        /* clean the previous command in the terminal */
			for(i = 0; i < 256; i++){
			    printf("%c", '\b');
			    printf("%c", ' ');
			    printf("%c", '\b');
			}
                	printf(PROMPT_STRING);

                        strcpy((char *)cmd_line_buffer,
                               (char *)(&history_buffer[history_buf_wlk_idx][0]));
                        printf("%s", cmd_line_buffer);

                        p = (char *)cmd_line_buffer;
                        while (*p++ != '\0') ;  // wait for '\r' pressed

                        continue;
                    }
                }
            }

            if ('\r' == ch) {
                *p = 0;
                ch = cmd_line_buffer[0];
                /* at least command-like */
                if (((ch >= 'A') && (ch <= 'Z')) || ((ch >= 'a') && (ch <= 'z'))) {
                    strcpy(history_buffer[history_buf_idx], cmd_line_buffer);
                    history_buf_idx++;
                    if (history_buf_idx >= HISTORY_CMD_MUX) {
                        history_buf_idx = 0;
                    }
                    history_buf_wlk_idx = history_buf_idx;
                    cmds_total++;

                    cmd_line_process(cmd_line_buffer, &argc, argvs);
                    if (0 != argc) {
                        for (cmd = cmd_start; cmd < cmd_end; cmd++) {
                            if (0 == strcmp(argvs[0], cmd->name)) {
                                break;
                            }
                        }

                        if (cmd < cmd_end) {
                            cmd->func(argc, argvs);
                        } else {
                            printf("\nInvalid command. \n");
                        }
                    }
                }

                p = (char *)(&cmd_line_buffer[0]);
                printf(PROMPT_STRING);
                continue;
            }

            p++;
        }
    }

    return 0;
}

int do_help(unsigned int argc, char *argvs[])
{
    shell_cmd_p cmd = (shell_cmd_p) __shell_cmds_beginning;

    if (1 == argc) {
        printf("\n");
        for (cmd = (shell_cmd_p) __shell_cmds_beginning;
             cmd < ((shell_cmd_p) __shell_cmds_end); cmd++) {
            printf("%s\n", cmd->short_usage);
        }
    }

    if (2 == argc) {
        cmd = (shell_cmd_p) __shell_cmds_beginning;
        for (; cmd < ((shell_cmd_p) __shell_cmds_end); cmd++) {
            if (0 == strcmp(argvs[1], cmd->name)) {
                break;
            }
        }

        if (cmd < ((shell_cmd_p) __shell_cmds_end)) {
            printf("\n%s\n", cmd->short_usage);
        } else {
            printf("%s is not a valid command.\n", argvs[1]);
        }
    }

    return 0;
}

SHELL_CMD(help, do_help, "help\t\tHelp commond\t\tUsage: help [cmd_name]")
