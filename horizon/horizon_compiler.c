#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "horizon_compiler.h"

// Internal functions

/* locate_forward, locate_backward and str_replace taken from https://stackoverflow.com/a/12546318 */

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

// str_replace(haystack, haystacksize, oldneedle, newneedle) --
//  Search haystack and replace all occurences of oldneedle with newneedle.
//  Resulting haystack contains no more than haystacksize characters (including the '\0').
//  If haystacksize is too small to make the replacements, do not modify haystack at all.
//
// RETURN VALUES
// str_replace() returns haystack on success and NULL on failure. 
// Failure means there was not enough room to replace all occurences of oldneedle.
// Success is returned otherwise, even if no replacement is made.
static char *str_replace(char *haystack, size_t haystacksize, const char *oldneedle, const char *newneedle)
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

// Note: This function returns a pointer to a substring of the original string.
// If the given string was allocated dynamically, the caller must not overwrite
// that pointer with the returned value, since the original pointer must be
// deallocated using the same allocator with which it was allocated.  The return
// value must NOT be deallocated using free() etc.
static char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

// End internal functions

// Parse program to build the symbol table and check for errors
// Returns the pointer to a newly allocated program_t
// If there are any errors, they will be added to the err_array, as many as
// err_array_size allows. The program_t return will have the total error count.
// After running horizon_parse, run horizon_free on the pointer to free its
// allocated memory, regardless of if the program was error-free or not.
program_t *horizon_parse(FILE *fd, error_t *err_array, int err_array_size)
{
    fseek(fd, 0, SEEK_END);
    int size = ftell(fd);
    rewind(fd);

    program_t program = { 0 };
    char *program_buf = malloc(size + 1);
    char *buf = malloc(size + 1);

    program.lines_buf = program_buf;
    size_t n_lines = 1000;
    program.lines = malloc(sizeof(char*) * n_lines);

    // Read whole program in
    int i = 0;
    program.len_lines = 0;
    while (!feof(fd))
    {
        // Read a line
        int c;
        int j = 0;
        while ((c = getc(fd)) != '\n' && c != EOF)
        {
            buf[j++] = toupper(c);
            i++;

            // Cut off comments, but keep reading the line
            if (c == ';')
                buf[j - 1] = '\0';

            // TODO: Name of program, description of program
        }
        buf[j] = 0;

        if (program.len_lines >= n_lines)
        {
            n_lines += 1000;
            program.lines = realloc(program.lines, sizeof(char*) * n_lines);
        }
        strcpy(&program_buf[i - j], buf);
        program.lines[program.len_lines++] = trimwhitespace(&program_buf[i - j]);
        i++;
    }

    free(buf);

    // First Pass: tokenize, get section, consts, handle built-in macros and @REP
    // blocks, handle labels, check instructions

    // Success
    program_t *ret = malloc(sizeof(program_t));
    *ret = program;
    return ret;
}

// Frees memory allocated by horizon_parse
void horizon_free(program_t *program)
{
    if (!program)
        return;
    if (program->lines_buf)
        free(program->lines_buf);
    if (program->lines)
        free(program->lines);
    if (program->line_executable)
        free(program->line_executable);
    if (program->code)
        free(program->code);
    free(program);
    return;
}

// Parse the program at the given src_filepath and save the result in the given
// destination path dst_filename.
// The options struct changes the way the procedure operates
void horizon_compile(const char *dst_filename, const char *src_filename, struct horizon_compiler_opt *options)
{
    horizon_perror(HOR_ERR_NOT_IMPLEMENTED);
    return;
}

// Print formatted error message to the console
void horizon_perror(int error)
{
    printf("Error: ");

    switch (error)
    {
        // No error
        case HOR_OK:
            printf("\r       \r");
            break;

        // Implementation
        case HOR_ERR_NOT_IMPLEMENTED:
            printf("not implemented\n");
            fflush(stdout);
            break;

        // Parsing

        // Compiling

        // Runtime
        default:
            printf("unknown error code\n");
    }
}
