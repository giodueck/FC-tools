#include <stdio.h>
#include "FC/overture.h"
#include "fcerrors.h"

int run(int argc, char **argv)
{
    return ERR_NOT_IMPLEMENTED;
}

void parse_and_run(const char *filename)
{
    FILE *fd = fopen(filename, "r");

    overture_init();
    int ret = overture_set_program(fd);
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
}

int compile(int argc, char **argv)
{
    return ERR_NOT_IMPLEMENTED;
}

int debug(int argc, char **argv)
{
    return ERR_NOT_IMPLEMENTED;
}
