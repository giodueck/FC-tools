#include <string.h>

#include "bp_creator.h"
#include "helpers.h"

// Replace placeholder strings in a blueprint string with int32 data.
// Leftover spots are not covered by this function, they must be set to
// a blank value in data.
char *bp_replace(char *bp_str_in, placeholder_func_t placeholder, int32_t *data, size_t len_data)
{
    char *bp_str_out = NULL;
    // The first char is ignored, that is the version number and always '0' for
    // Factorio versions through 1.1
    size_t b64decoded_size = base64_decode_len(bp_str_in + 1, strlen(bp_str_in + 1));

    // Decode input BP string
    // Write to a tmpfile
    // Decompress decoded data into a tmpfile
    // Modify json and copy into a tmpfile
    // Compress json into a tmpfile
    // Encode compressed data into new BP string

    return bp_str_out;
}
