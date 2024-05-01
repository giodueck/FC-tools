#include <stdio.h>
#include <stdlib.h>
#include "bp_creator.h"
#include "rom_bp_strings.h"

int main()
{
    for (int i = 0; i < 10; i++)
    {
        char buf[100] = { 0 };
        rom_11_placeholder(buf, i);
        printf("%s => %d\n", buf, (int)is_rom_11_placeholder(atoi(buf)));
    }
    printf("%d => %d\n", 12345, (int)is_rom_11_placeholder(12345));

    int32_t data[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    char *bp_str_out = bp_replace(rom_11_bit, is_rom_11_placeholder, data, 10);
    printf("%s\n", bp_str_out);

    free(bp_str_out);
    return 0;
}
