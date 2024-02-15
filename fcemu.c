#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

#include "FC/overture.h"

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

void initialize();

void handle_inputs();

void render();

void simulate();

int main()
{
    FILE *fd = fopen("fib.txt", "r");

    overture_init();
    overture_parse(fd);
    overture_print();
    overture_free();
    overture_quit();

    fclose(fd);
    return 0;
}
