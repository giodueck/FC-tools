#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>

#include "fccommands.h"
#include "fcerrors.h"

/* Basic scaffolding:
 *  Initialization (Horizon ISA, Horizon memory layout, no screen)
 *  Input handling: gdb inspired
 *      Run
 *      Step
 *      Set breakpoint
 *      Continue
 *      Next (don't follow, just execute function calls)
 *  Simulation
 *      Expand macros
 *      Interpret instructions (no need to compile)
 *
 * Planned structure:
 *  Initialization
 *      Load configuration to use
 *          ISA
 *          Memory Layout
 *          Output devices
 *          Input devices
 *  Input handling
 *      Input virtual (debugger)
 *      Input emulated (emulated input device)
 *  Simulation
 *      Interpret intructions
 *      Associate abstract instructions to their translations
 *      Associate instructions to their line
 */

int help(int argc, char **argv);

const int len_commands = 4;
const char *commands[] = { "run", "compile", "debug", "help" };
int (*command_functions[])(int, char **) = { run, compile, debug, help };
const char *command_help[] = { "Run a program", "Compile a program", "Compile and run or step through the execution", "Print this help menu" };

void initialize();

void handle_inputs();

void render();

void simulate();

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
        print_help(-1);
    else if (strcmp(argv[1], "help")) // error with the command
    {
        print_help(-1);
    }
    return 0;
}

int main(int argc, char **argv)
{
    int ret = ERR_INVALID_COMMAND;
    if (argc > 1)
    {

        for (int i = 0; i < len_commands && ret == ERR_INVALID_COMMAND; i++)
        {
            if (strcmp(argv[1], commands[i]) == 0)
                ret = command_functions[i](argc, argv);
        }

        if (ret == ERR_NOT_IMPLEMENTED)
        {
            printf("This command is not yet implemented\n");
        }
    }

    if (ret == ERR_INVALID_COMMAND)
    {
        help(argc, argv);
    }

    return ret;
}
