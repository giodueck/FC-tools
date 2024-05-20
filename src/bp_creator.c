#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zlib.h>

#include "bp_creator.h"
#include "helpers.h"
#include "rom_bp_strings.h"

// Replace placeholder strings in a blueprint string with int32 data.
// Leftover spots are not covered by this function, they must be set to
// a blank value in data.
char *bp_replace(const char *bp_str_in, placeholder_index_func_t is_placeholder, int32_t *data, size_t len_data)
{
    char *bp_str_out = NULL;
    // The first char is ignored, that is the version number and always '0' for
    // Factorio versions through 1.1
    size_t b64_decoded_size = base64_decode_len(bp_str_in + 1, strlen(bp_str_in + 1));

    // Decode input BP string
    char *b64_decoded_data = malloc(b64_decoded_size);
    int res = base64_decode(b64_decoded_data, bp_str_in + 1);
    if (res != 0)
    {
        free(b64_decoded_data);
        return NULL;
    }

    // Write to a tmpfile
    FILE *fd_def_in = tmpfile();
    if (fd_def_in == NULL)
    {
        perror("bp_replace tmpfile");
        free(b64_decoded_data);
        return NULL;
    }

    res = fwrite(b64_decoded_data, 1, b64_decoded_size, fd_def_in);
    if (res != b64_decoded_size)
    {
        perror("bp_replace fwrite");
        free(b64_decoded_data);
        fclose(fd_def_in);
        return NULL;
    }
    free(b64_decoded_data);

    // Decompress decoded data into a tmpfile
    rewind(fd_def_in);
    FILE *fd_json_in = tmpfile();
    if (fd_json_in == NULL)
    {
        perror("bp_replace tmpfile");
        fclose(fd_def_in);
        return NULL;
    }

    res = inf(fd_def_in, fd_json_in);
    if (res != Z_OK)
    {
        zerr(res);
        fclose(fd_json_in);
        fclose(fd_def_in);
        return NULL;
    }
    fclose(fd_def_in);

    // Modify json and copy into a tmpfile
    rewind(fd_json_in);
    // make changes and write into new file
    FILE *fd_json_out = tmpfile();
    if (fd_json_out == NULL)
    {
        perror("bp_replace tmpfile");
        fclose(fd_json_in);
        return NULL;
    }

    char buf[BUFSIZ] = { 0 };
    char numbuf[128] = { 0 };
    int num_i = 0;
    while ((res = fread(buf, 1, BUFSIZ, fd_json_in)) > 0)
    {
        // Replacement algorithm:
        // - Read a character
        // - If char is '-' or '0'-'9': add to numbuf and keep count
        // - Else:
        //   - If numbuf is not empty, test if placeholder
        //     - If numbuf is a placeholder, write to out the appropriate program word
        //     - Else, write numbuf to out
        //   - Else, write char to out
        //
        // Potential improvement: write both to buffers, when the char buffer fills or a numeric char
        // is read, flush the text buffer and fill the numbuf, when a non numeric char is found, do the
        // above. This avoids unneeded file I/O
        for (int i = 0; i < res; i++)
        {
            switch (buf[i])
            {
                case '-':
                    if (num_i)
                    {
                        // Something is wrong if we ever get here, but that is not for us to worry about.
                        // This is to avoid more errors, and if a number is written to output here, it will
                        // get mixed up with the previous one and be wrong regardless.
                        fwrite(numbuf, 1, num_i, fd_json_out);
                        num_i = 0;
                    }

                    // fallthrough
                case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                    numbuf[num_i++] = buf[i];
                    break;
                default:
                    if (num_i)
                    {
                        numbuf[num_i] = 0;
                        int32_t num = atoi(numbuf);
                        int64_t placeholder_i = is_placeholder(num);

                        // Put the relevant data word, or 0 if past data length
                        if (placeholder_i >= 0)
                        {
                            if (placeholder_i < len_data)
                                fprintf(fd_json_out, "%d", data[placeholder_i]);
                            else
                                putc('0', fd_json_out);
                        }
                        else
                            fwrite(numbuf, 1, num_i, fd_json_out);

                        num_i = 0;
                    }

                    putc(buf[i], fd_json_out);
                    break;
            }
        }
    }
    if (feof(fd_json_in))
    {
        fclose(fd_json_in);
    }
    else
    {
        perror("bp_replace fread");
        fclose(fd_json_in);
        fclose(fd_json_out);
        return NULL;
    }

    // Compress json into a tmpfile
    rewind(fd_json_out);
    FILE *fd_def_out = tmpfile();
    if (fd_def_out == NULL)
    {
        perror("bp_replace tmpfile");
        fclose(fd_json_out);
        return NULL;
    }

    res = def(fd_json_out, fd_def_out, 9);
    if (res != Z_OK)
    {
        zerr(res);
        fclose(fd_def_out);
        fclose(fd_json_out);
        return NULL;
    }
    fclose(fd_json_out);

    // Encode compressed data into new BP string
    fseek(fd_def_out, 0, SEEK_END);
    long def_out_size = ftell(fd_def_out);
    rewind(fd_def_out);

    size_t bp_str_out_size = base64_encode_len(def_out_size);
    bp_str_out = malloc(bp_str_out_size + 1);

    char *buf_def_out = malloc(def_out_size + 2);
    res = fread(buf_def_out, 1, def_out_size, fd_def_out);
    fclose(fd_def_out);

    base64_encode(bp_str_out + 1, buf_def_out, def_out_size);
    bp_str_out[0] = '0';
    free(buf_def_out);

    return bp_str_out;
}

char *bp_set_name_desc(const char *bp_str_in, const char *placeholder_name, const char *new_name, const char *placeholder_desc, const char *new_desc)
{
    char *bp_str_out = NULL;
    // The first char is ignored, that is the version number and always '0' for
    // Factorio versions through 1.1
    size_t b64_decoded_size = base64_decode_len(bp_str_in + 1, strlen(bp_str_in + 1));

    // Decode input BP string
    char *b64_decoded_data = malloc(b64_decoded_size);
    int res = base64_decode(b64_decoded_data, bp_str_in + 1);
    if (res != 0)
    {
        free(b64_decoded_data);
        return NULL;
    }

    // Write to a tmpfile
    FILE *fd_def_in = tmpfile();
    if (fd_def_in == NULL)
    {
        perror("bp_replace tmpfile");
        free(b64_decoded_data);
        return NULL;
    }

    res = fwrite(b64_decoded_data, 1, b64_decoded_size, fd_def_in);
    if (res != b64_decoded_size)
    {
        perror("bp_replace fwrite");
        free(b64_decoded_data);
        fclose(fd_def_in);
        return NULL;
    }
    free(b64_decoded_data);

    // Decompress decoded data into a tmpfile
    rewind(fd_def_in);
    FILE *fd_json_in = tmpfile();
    if (fd_json_in == NULL)
    {
        perror("bp_replace tmpfile");
        fclose(fd_def_in);
        return NULL;
    }

    res = inf(fd_def_in, fd_json_in);
    if (res != Z_OK)
    {
        zerr(res);
        fclose(fd_json_in);
        fclose(fd_def_in);
        return NULL;
    }
    fclose(fd_def_in);

    // Modify json and copy into a tmpfile
    rewind(fd_json_in);
    // make changes and write into new file
    FILE *fd_json_out = tmpfile();
    if (fd_json_out == NULL)
    {
        perror("bp_replace tmpfile");
        fclose(fd_json_in);
        return NULL;
    }

    // Get filesize
    fseek(fd_json_in, 0, SEEK_END);
    size_t fsize = ftell(fd_json_in);
    char *buf = malloc(2 * fsize);
    memset(buf, 0, 2 * fsize);

    // Read in whole file
    rewind(fd_json_in);
    int c = 0;
    int i = 0;
    while ((c = fgetc(fd_json_in)) != EOF)
        buf[i++] = c;
    buf[i] = 0;

    // Search and replace
    if (new_desc) str_replace(buf, 2 * fsize, placeholder_desc, new_desc);
    if (new_name) str_replace(buf, 2 * fsize, placeholder_name, new_name);

    // Write out file
    for (int i = 0; buf[i] != 0; i++)
        fputc(buf[i], fd_json_out);
    free(buf);

    // Compress json into a tmpfile
    rewind(fd_json_out);
    FILE *fd_def_out = tmpfile();
    if (fd_def_out == NULL)
    {
        perror("bp_replace tmpfile");
        fclose(fd_json_out);
        return NULL;
    }

    res = def(fd_json_out, fd_def_out, 9);
    if (res != Z_OK)
    {
        zerr(res);
        fclose(fd_def_out);
        fclose(fd_json_out);
        return NULL;
    }
    fclose(fd_json_out);

    // Encode compressed data into new BP string
    fseek(fd_def_out, 0, SEEK_END);
    long def_out_size = ftell(fd_def_out);
    rewind(fd_def_out);

    size_t bp_str_out_size = base64_encode_len(def_out_size);
    bp_str_out = malloc(bp_str_out_size + 1);

    char *buf_def_out = malloc(def_out_size + 2);
    res = fread(buf_def_out, 1, def_out_size, fd_def_out);
    fclose(fd_def_out);

    base64_encode(bp_str_out + 1, buf_def_out, def_out_size);
    bp_str_out[0] = '0';
    free(buf_def_out);

    return bp_str_out;
}
