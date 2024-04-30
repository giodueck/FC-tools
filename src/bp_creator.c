#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zlib.h>

#include "bp_creator.h"
#include "helpers.h"

// Replace placeholder strings in a blueprint string with int32 data.
// Leftover spots are not covered by this function, they must be set to
// a blank value in data.
char *bp_replace(const char *bp_str_in, placeholder_func_t placeholder, int32_t *data, size_t len_data)
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
    while ((res = fread(buf, 1, BUFSIZ, fd_json_in)) > 0)
    {
        // placeholder
        int reswrite = fwrite(buf, 1, res, fd_json_out);
        if (reswrite == 0)
        {
            perror("bp_replace fwrite");
            fclose(fd_json_in);
            fclose(fd_json_out);
            return NULL;
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
