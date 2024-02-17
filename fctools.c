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

extern const int len_commands;
extern const char *commands[];
extern int (*command_functions[])(int, char **);
extern const char *command_help[];
extern int command;
extern int error;

void initialize();

void handle_inputs();

void render();

void simulate();


int main(int argc, char **argv)
{
    int ret = ERR_INVALID_COMMAND;
    if (argc > 1)
    {

        for (int i = 0; i < len_commands && ret == ERR_INVALID_COMMAND; i++)
        {
            if (strcmp(argv[1], commands[i]) == 0)
            {
                command = i;
                ret = command_functions[i](argc, argv);
                break;
            }
        }

        if (ret == ERR_NOT_IMPLEMENTED)
        {
            printf("This command is not yet implemented\n");
        }
    }

    if (ret == ERR_INVALID_COMMAND)
    {
        error = ERR_INVALID_COMMAND;
        help(argc, argv);
    }

    return ret;
}
