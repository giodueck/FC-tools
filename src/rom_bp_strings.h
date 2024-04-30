#ifndef ROM_BP_STRINGS_H
#define ROM_BP_STRINGS_H

const extern char *rom_11_bit;

typedef int (*placeholder_func_t)(char *dest, int i);

// Sets dest to the placeholder string for the ith element in the json of the blueprint
int rom_11_bp_placeholder(char dest[8], int i);

#endif // ROM_BP_STRINGS_H
