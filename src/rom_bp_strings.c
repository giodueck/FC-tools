#include <stdio.h>

const char *rom_11_bit = "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";

// Sets dest to the placeholder string for the ith element in the json of the blueprint
int rom_11_bp_placeholder(char dest[8], int i)
{
    sprintf(dest, "%d", (i + 10000) * 1000 + 123);
    return 0;
}