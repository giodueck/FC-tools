#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FC/overture.h"
#include "fcerrors.h"
#include "fccommands.h"

const int len_commands = 4;
const char *commands[] = { "run", "compile", "debug", "help" };
int (*command_functions[])(int, char **) = { run, compile, debug, help };
const char *command_help[] = { "Run a program", "Compile a program", "Compile and run or step through the execution", "Print this help menu" };

static char missing_flag = 0;
static int error = 0;

// Parse and compile program, then run it
void parse_and_run(FILE *fd);

// Run a program
int run(int argc, char **argv)
{
    if (argc < 3)
    {
        error = ERR_NO_FILE_GIVEN;
        help(argc, argv);
        return error;
    }

    FILE *fd;
    if ((fd = fopen(argv[2], "r")) != NULL)
    {
        parse_and_run(fd);
    } else
    {
        perror("run");
    }

    return EXIT_SUCCESS;
}

void parse_and_run(FILE *fd)
{
    overture_init();
    int ret = overture_set_program(fd);

    if (ret == 0)
    {
        printf("Setting end: %d\n", overture_set_end(-1));
        printf("Running program...");
        fflush(stdout);
        overture_run(RUN);
        printf("done\n");
        overture_register_map_t reg = overture_get_registers();
        printf("R0: %d\nR1: %d\nR2: %d\nR3: %d\nR4: %d\nR5: %d\n", reg.r0, reg.r1, reg.r2, reg.r3, reg.r4, reg.r5);
    }

    overture_free();
    overture_quit();
}

int compile(int argc, char **argv)
{
    return ERR_NOT_IMPLEMENTED;
}

int debug(int argc, char **argv)
{
    return ERR_NOT_IMPLEMENTED;
}

void print_help(int function)
{
    if (function < 0)
    {
        printf("Usage: fctools <command> [command options]\n");
        printf("\nAvailable commands:\n");
        for (int i = 0; i < len_commands; i++)
        {
            printf("    %s\r\t\t%s\n", commands[i], command_help[i]);
        }
    }
}

int help(int argc, char **argv)
{
    if (argc < 2 || (argc >= 2 && strcmp(argv[1], "help") == 0))
    {
        print_help(-1);
    } else if (strcmp(argv[1], "help")) // error with the command
    {
        print_help(-1);

        if (error != NO_ERR)
        {
            printf("\n");
            switch (error)
            {
            case ERR_NO_FILE_GIVEN:
                printf("Error: No file given\n");
                break;
            default:
                break;
            }
        }
    }
    return 0;
}
