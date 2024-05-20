#ifndef ROM_BP_STRINGS_H
#define ROM_BP_STRINGS_H

#include <stdint.h>

extern const int rom_11_capacity;
extern const char *rom_11_name;
extern const char *rom_11_desc;
extern const char *rom_11_bit;

typedef int (*placeholder_func_t)(char *dest, int i);
typedef int64_t (*placeholder_index_func_t)(int placeholder);

// Sets dest to the placeholder string for the ith element in the json of the blueprint
int rom_11_placeholder(char dest[8], int i);

// Returns the index of the placeholder if valid, or -1 of not
int64_t is_rom_11_placeholder(int placeholder);

extern const int rom_12_capacity;
extern const char *rom_12_name;
extern const char *rom_12_desc;
extern const char *rom_12_bit;

typedef int (*placeholder_func_t)(char *dest, int i);
typedef int64_t (*placeholder_index_func_t)(int placeholder);

// Sets dest to the placeholder string for the ith element in the json of the blueprint
int rom_12_placeholder(char dest[8], int i);

// Returns the index of the placeholder if valid, or -1 of not
int64_t is_rom_12_placeholder(int placeholder);

#endif // ROM_BP_STRINGS_H
