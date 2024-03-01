#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "horizon_compiler.h"
#include "horizon_parser.h"

// Internal helper functions

static void horizon_syntax_error(const char *message, int line_minus_one)
{
    printf("Error on line %d:\n\t%s\n", line_minus_one + 1, message);
}

int horizon_symbol_exists(program_t program, const char *token)
{
    for (int i = 0; i < program.len_symbols; i++)
    {
        if (strcmp(token, program.symbols[i].name) == 0)
            return 1;
    }
    return 0;
}

// End internal helper functions

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

    program_t program = { ARCH_HORIZON };
    char *program_buf = malloc(size + 1);

    program.lines_buf = program_buf;
    size_t n_lines = 1000;
    program.lines = malloc(sizeof(char*) * n_lines);

    // Read whole program in
    int i = 0;
    program.len_lines = 0;
    while (!feof(fd))
    {
        fread(program_buf, 1, size, fd);
    }

    // Allocate the needed program space
    program.line_executable = malloc(sizeof(int) * program.len_lines);
    int symbol_space = 100;
    program.symbols = malloc(sizeof(symbol_t) * symbol_space);

    uint32_t num;
    int retval;

    for (int i = 0; i < 17; i++)
    {
        match_whitespace(&program_buf);
        retval = match_register(&num, &program_buf);
        if (retval != 0)
        {
            parser_perror("horizon_parse", retval);
            break;
        }
        printf("%u\n", num);
        retval = match_newline(&program_buf);
        if (retval != 0)
        {
            if (retval == ERR_EOF)
                break;
            parser_perror("horizon_parse", retval);
            break;
        }
    }

    printf("unparsed text: %s\n", program_buf);

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
