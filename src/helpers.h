#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>

// Replace substrings in a string
// haystack: original string
// haystacksize: size of haystack
// oldneedle: substring to replace
// newneedle: substring to replace oldneedle with
char *str_replace(char *haystack, size_t haystacksize, const char *oldneedle, const char *newneedle);

// Base64 decode
// decoded: destination buffer for decoded bytes data. Assumed to be big enough, does not null-terminate.
// b64_data: null-terminated string of base 64 data to decode.
int base64_decode(char *decoded, char *b64_data);

// Base64 encode
// encoded: destination buffer for encoded base 64 null-terminated string. Assumed to be big enough.
// bytes_data: bytes buffer to encode, not necessarily null-terminated.
// bytes_len: length of bytes_data.
int base64_encode(char *encoded, char *bytes_data, size_t bytes_len);

// Length of decoded bytes data for the given length of base 64 characters.
// A pointer to the data is passed to account for any padding.
// b64_data: base 64 encoded string.
// b64_len: length of base 64 encoded string.
size_t base64_decode_len(char *b64_data, size_t b64_len);

// Length of encoded base 64 string for the giveb length of bytes data.
// bytes_len: length of bytes data.
size_t base64_encode_len(size_t bytes_len);

#endif // HELPERS_H
