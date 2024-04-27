#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helpers.h"

// locate_forward: compare needle_ptr and read_ptr to see if a match occured
// needle_ptr is updated as appropriate for the next call
// return 1 if match occured, false otherwise
static inline int locate_forward(char **needle_ptr, char *read_ptr, const char *needle, const char *needle_last)
{
    if (**needle_ptr == *read_ptr) {
        (*needle_ptr)++;
        if (*needle_ptr > needle_last) {
            *needle_ptr = (char *)needle;
            return 1;
        }
    }
    else
        *needle_ptr = (char *)needle;
    return 0;
}

// locate_backward: compare needle_ptr and read_ptr to see if a match occured
// needle_ptr is updated as appropriate for the next call
// return 1 if match occured, 0 otherwise
static inline int locate_backward(char **needle_ptr, char *read_ptr, const char *needle, const char *needle_last)
{
    if (**needle_ptr == *read_ptr) {
        (*needle_ptr)--;
        if (*needle_ptr < needle) {
            *needle_ptr = (char *)needle_last;
            return 1;
        }
    }
    else
        *needle_ptr = (char *)needle_last;
    return 0;
}

// Replace substrings in a string
// haystack: original string
// haystacksize: size of haystack
// oldneedle: substring to replace
// newneedle: substring to replace oldneedle with
char *str_replace(char *haystack, size_t haystacksize, const char *oldneedle, const char *newneedle)
{
    size_t oldneedle_len = strlen(oldneedle);
    size_t newneedle_len = strlen(newneedle);
    char *oldneedle_ptr;    // locates occurences of oldneedle
    char *read_ptr;         // where to read in the haystack
    char *write_ptr;        // where to write in the haystack
    const char *oldneedle_last =  // the last character in oldneedle
        oldneedle +
        oldneedle_len - 1;

    // Case 0: oldneedle is empty
    if (oldneedle_len == 0)
        return haystack;     // nothing to do; define as success

    // Case 1: newneedle is not longer than oldneedle
    if (newneedle_len <= oldneedle_len) {
        // Pass 1: Perform copy/replace using read_ptr and write_ptr
        for (oldneedle_ptr = (char *)oldneedle,
            read_ptr = haystack, write_ptr = haystack;
            *read_ptr != '\0';
            read_ptr++, write_ptr++)
        {
            *write_ptr = *read_ptr;
            int found = locate_forward(&oldneedle_ptr, read_ptr,
                        oldneedle, oldneedle_last);
            if (found)  {
                // then perform update
                write_ptr -= oldneedle_len;
                memcpy(write_ptr+1, newneedle, newneedle_len);
                write_ptr += newneedle_len;
            }
        }
        *write_ptr = '\0';
        return haystack;
    }

    // Case 2: newneedle is longer than oldneedle
    else {
        size_t diff_len =       // the amount of extra space needed
            newneedle_len -     // to replace oldneedle with newneedle
            oldneedle_len;      // in the expanded haystack

        // Pass 1: Perform forward scan, updating write_ptr along the way
        for (oldneedle_ptr = (char *)oldneedle, read_ptr = haystack, write_ptr = haystack;
            *read_ptr != '\0' && write_ptr < haystack + haystacksize;
            read_ptr++, write_ptr++)
        {
            int found = locate_forward(&oldneedle_ptr, read_ptr,
                        oldneedle, oldneedle_last);
            if (found) {
                // then advance write_ptr
                write_ptr += diff_len;
            }
        }
        if (write_ptr >= haystack+haystacksize)
            return NULL; // no more room in haystack

        // Pass 2: Walk backwards through haystack, performing copy/replace
        for (oldneedle_ptr = (char *)oldneedle_last;
            write_ptr >= haystack;
            write_ptr--, read_ptr--)
        {
            *write_ptr = *read_ptr;
            int found = locate_backward(&oldneedle_ptr, read_ptr,
                        oldneedle, oldneedle_last);
            if (found) {
                // then perform replacement
                write_ptr -= diff_len;
                memcpy(write_ptr, newneedle, newneedle_len);
            }
        }
        return haystack;
    }
}

// Lookup table for 6-bit keys
static const char base64_encode_lookup[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

// Base64 decode
// decoded: destination buffer for decoded bytes data. Assumed to be big enough, does not null-terminate.
// b64_data: null-terminated string of base 64 data to decode.
int base64_decode(char *decoded, char *b64_data);

// Base64 encode
// encoded: destination buffer for encoded base 64 null-terminated string. Assumed to be big enough.
// bytes_data: bytes buffer to encode, not necessarily null-terminated.
// bytes_len: length of bytes_data.
int base64_encode(char *encoded, char *bytes_data, size_t bytes_len)
{
    size_t len = base64_encode_len(bytes_len);
    int j = 0;
    int ti = 0;
    char triplet[3] = { 0 };

    // Iterate through source array and group triplets
    // When a triplet is complete, encode it and empty the triplet array
    for (int i = 0; i < bytes_len; i++)
    {
        triplet[ti++] = bytes_data[i];
        if (ti == 3)
        {
            encoded[j++] = base64_encode_lookup[triplet[0] >> 2];
            encoded[j++] = base64_encode_lookup[(triplet[0] & 0x3) << 4 | (triplet[1] >> 4)];
            encoded[j++] = base64_encode_lookup[(triplet[1] & 0xF) << 2 | (triplet[2] >> 6)];
            encoded[j++] = base64_encode_lookup[triplet[2] & 0x3F];
            ti = 0;
        }
    }

    // When the end is reached and a partial triplet is still unencoded, encode with padding
    switch (ti)
    {
        case 1:
            encoded[j++] = base64_encode_lookup[triplet[0] >> 2];
            encoded[j++] = base64_encode_lookup[(triplet[0] & 0x3) << 4];
            encoded[j++] = '=';
            encoded[j++] = '=';
            break;
        case 2:
            encoded[j++] = base64_encode_lookup[triplet[0] >> 2];
            encoded[j++] = base64_encode_lookup[(triplet[0] & 0x3) << 4 | (triplet[1] >> 4)];
            encoded[j++] = base64_encode_lookup[(triplet[1] & 0xF) << 2];
            encoded[j++] = '=';
            break;
        default:
            break;
    }

    encoded[j] = 0;

    return 0;
}

// Length of decoded bytes data for the given length of base 64 characters.
// A pointer to the data is passed to account for any padding.
// For an invalid length, returns -1.
// b64_data: base 64 encoded string.
// b64_len: length of base 64 encoded string.
size_t base64_decode_len(char *b64_data, size_t b64_len)
{
    // Encoded string must be divisible by 4
    if (b64_len & 0x3)
    {
        return -1;
    }

    // At most 3*len/4 bytes long
    size_t upper = 3 * (b64_len >> 2);

    // Check padding:
    // xx== : 1 byte  => -2
    // xxx= : 2 bytes => -1
    // xxxx : 3 bytes =>  0
    for (int i = 0; i < 3; i++)
    {
        upper -= (b64_data[b64_len - i] == '=');
    }

    return upper;
}

// Length of encoded base 64 string for the giveb length of bytes data.
// bytes_len: length of bytes data.
size_t base64_encode_len(size_t bytes_len)
{
    return ((bytes_len + 2) / 3 * 4);
}

