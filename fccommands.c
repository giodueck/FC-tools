#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "FC/overture.h"
#include "fcerrors.h"
#include "fccommands.h"

const int len_commands = 4;
const char *commands[] = { "run", "compile", "debug", "help" };
int (*command_functions[])(int, char **) = { run, compile, debug, help };

const char *command_help[] = { "Run a program.", "Compile a program.", "Compile and run or step through the execution.", "Print this help menu. Use \"help <command>\" for help on that command." };

void (*command_help_functions[])(void) = { run_help, compile_help, debug_help, help_help };

// Error reporting
static char missing_arg = 0;
int error = 0;
int command = -1;

// Options
int opt;
extern char *optarg;
extern int optopt;

const char *run_optstring = ":f:";
const char *run_req_opt = "y";
const char *run_opt_help[] = { "Filename of the program" };

// Parse and compile program, then run it
void parse_and_run(FILE *fd);

// Run a program
int run(int argc, char **argv)
{
    char filename[BUFSIZ] = "";

    while ((opt = getopt(argc, argv, run_optstring)) != -1)
    {
        switch (opt)
        {
        case 'f':
            strcpy(filename, optarg);
            break;
        case ':':
            error = ERR_NO_ARG;
            help(argc, argv);
            return error;
        default: // ?
            error = ERR_BAD_OPTION;
            help(argc, argv);
            return error;
        }
    }

    // Error checking
    if (strlen(filename) == 0)
    {
        error = ERR_MISSING_OPT;
        missing_arg = 'f';
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

void run_help()
{
    char curopt = 0;
    int nopt = 0;

    printf("Usage: fctools run [options]\n");
    printf("\nAvailable options:\n");
    for (int i = 0; i < strlen(run_optstring); i++)
    {
        switch (run_optstring[i])
        {
        case ':': case '-': case '+': case ';':
            continue;
        default:
            printf("    -%c %s\r\t\t\t%s.\n", run_optstring[i], (run_optstring[i+1] == ':' ? "<argument>" : ""), run_opt_help[nopt]);
            if (run_req_opt[nopt] == 'y')
            {
                printf("\t\t\tThis option is required.\n");
            }
            nopt++;
            break;
        }
    }
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

void compile_help()
{
}

int debug(int argc, char **argv)
{
    return ERR_NOT_IMPLEMENTED;
}

void debug_help()
{
}

void print_help(int function)
{
    if (function < 0 || strcmp(commands[function], "help") == 0)
    {
        printf("Usage: fctools <command> [command options]\n");
        printf("\nAvailable commands:\n");
        for (int i = 0; i < len_commands; i++)
        {
            printf("    %s\r\t\t%s\n", commands[i], command_help[i]);
        }
    }
}

void help_help()
{
}

int help(int argc, char **argv)
{
    // If called without commands or with the help command
    if (argc < 2 || (argc >= 2 && strcmp(argv[1], "help") == 0))
    {
        // If called like help <command>
        if (argc >= 3)
        {
            for (int i = 0; i < len_commands; i++)
            {
                if (strcmp(argv[2], commands[i]) == 0)
                {
                    command_help_functions[i]();
                    return NO_ERR;
                }
            }
        }
        print_help(-1);
    } else if (strcmp(argv[1], "help")) // error with the command
    {
        if (error != ERR_INVALID_COMMAND)
            print_help(command);

        if (error != NO_ERR)
        {
            switch (error)
            {
            case ERR_INVALID_COMMAND:
                printf("Error: Unknown command: %s\n", argv[1]);
                print_help(-1);
                return error;
            case ERR_BAD_OPTION:
                printf("Error: Unknown option: -%c\n", optopt);
                break;
            case ERR_NO_ARG:
                printf("Error: Option -%c requires an argument\n", optopt);
                break;
            case ERR_MISSING_OPT:
                printf("Error: Missing required option: %c\n", missing_arg);
                break;
            default:
                break;
            }
            printf("Use \"fctools help <command>\" for help on the available commands\n");
        }
    }
    return 0;
}
