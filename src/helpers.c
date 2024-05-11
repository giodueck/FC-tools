#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <zlib.h>
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
static const char base64_to_ascii[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Lookup table for 8-bit ascii keys
static const int ascii_to_base64[256] =
{
	/* ASCII table */
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
	64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
	64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

// Base64 decode
// Quirks: requires padding, allows no characters outside of the base64 alphabet, not even whitespace.
// On successful decoding returns 0, if an error was encountered with the data, -1.
// decoded: destination buffer for decoded bytes data. Assumed to be big enough, does not null-terminate.
// b64_data: null-terminated string of base 64 data to decode.
int base64_decode(char *decoded, const char *b64_data)
{
    int j = 0;
    int qi = 0;
    char quad[4] = { 0 };
    int pad = 0;

    for (int i = 0; b64_data[i] != '\0'; i++)
    {
        if (ascii_to_base64[(int) b64_data[i]] == 64 && b64_data[i] != '=')
            return -1;

        quad[qi++] = ascii_to_base64[(int) b64_data[i]];
        pad += (b64_data[i] == '=');

        if (qi == 4)
        {
            decoded[j++] = quad[0] << 2 | quad[1] >> 4;
            if (pad < 2) decoded[j++] = (quad[1] & 0xF) << 4 | quad[2] >> 2;
            if (pad < 1) decoded[j++] = (quad[2] & 0x3) << 6 | quad[3];
            qi = 0;

            if (pad) break;
        }
    }

    return 0;
}

// Base64 encode
// encoded: destination buffer for encoded base 64 null-terminated string. Assumed to be big enough.
// bytes_data: bytes buffer to encode, not necessarily null-terminated.
// bytes_len: length of bytes_data.
int base64_encode(char *encoded, const char *bytes_data, size_t bytes_len)
{
    int j = 0;
    int ti = 0;
    unsigned char triplet[3] = { 0 };

    // Iterate through source array and group triplets
    // When a triplet is complete, encode it and empty the triplet array
    for (int i = 0; i < bytes_len; i++)
    {
        triplet[ti++] = bytes_data[i];
        if (ti == 3)
        {
            encoded[j++] = base64_to_ascii[triplet[0] >> 2];
            encoded[j++] = base64_to_ascii[(triplet[0] & 0x3) << 4 | (triplet[1] >> 4)];
            encoded[j++] = base64_to_ascii[(triplet[1] & 0xF) << 2 | (triplet[2] >> 6)];
            encoded[j++] = base64_to_ascii[triplet[2] & 0x3F];
            ti = 0;
        }
    }

    // When the end is reached and a partial triplet is still unencoded, encode with padding
    switch (ti)
    {
        case 1:
            encoded[j++] = base64_to_ascii[triplet[0] >> 2];
            encoded[j++] = base64_to_ascii[(triplet[0] & 0x3) << 4];
            encoded[j++] = '=';
            encoded[j++] = '=';
            break;
        case 2:
            encoded[j++] = base64_to_ascii[triplet[0] >> 2];
            encoded[j++] = base64_to_ascii[(triplet[0] & 0x3) << 4 | (triplet[1] >> 4)];
            encoded[j++] = base64_to_ascii[(triplet[1] & 0xF) << 2];
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
size_t base64_decode_len(const char *b64_data, size_t b64_len)
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

// Length of encoded base 64 string for the given length of bytes data.
// bytes_len: length of bytes data.
size_t base64_encode_len(size_t bytes_len)
{
    return ((bytes_len + 2) / 3 * 4);
}

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int def(FILE *source, FILE *dest, int level)
{
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;

    /* compress until end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int inf(FILE *source, FILE *dest)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
void zerr(int ret)
{
    fputs("zpipe: ", stderr);
    switch (ret) {
    case Z_ERRNO:
        if (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        if (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        break;
    case Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        break;
    case Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        break;
    case Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        break;
    case Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}
