#ifndef BP_CREATOR_H
#define BP_CREATOR_H

#include <stdint.h>
#include <stdio.h>
#include "rom_bp_strings.h"

// Replace placeholder strings in a blueprint string with int32 data.
// Leftover spots are not covered by this function, they must be set to
// a blank value in data.
char *bp_replace(const char *bp_str_in, placeholder_index_func_t is_placeholder, int32_t *data, size_t len_data);

char *bp_set_name_desc(const char *bp_str_in, const char *placeholder_name, const char *new_name, const char *placeholder_desc, const char *new_desc);

#endif // BP_CREATOR_H
