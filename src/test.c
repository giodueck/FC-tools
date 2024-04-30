#include <stdio.h>
#include <stdlib.h>
#include "bp_creator.h"
#include "rom_bp_strings.h"

int main()
{
    char *bp_str_out = bp_replace(rom_11_bit, rom_11_bp_placeholder, NULL, 0);
    printf("%s\n", bp_str_out);

    free(bp_str_out);
    return 0;
}
