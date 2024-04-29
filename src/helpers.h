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

// zlib compression and decompression functions from:
/* zpipe.c: example of proper use of zlib's inflate() and deflate()
   Not copyrighted -- provided to the public domain
   Version 1.4  11 December 2005  Mark Adler */

// 128K
#define CHUNK 131072
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int def(FILE *source, FILE *dest, int level);

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int inf(FILE *source, FILE *dest);

/* report a zlib or i/o error */
void zerr(int ret);

#endif // HELPERS_H
