#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

#include "FC/overture.h"
#include "FC/program.h"

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
    int ret = overture_parse(fd);
    fclose(fd);

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

    return ret;
}
