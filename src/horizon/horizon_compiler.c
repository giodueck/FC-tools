#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "horizon_compiler.h"
#include "horizon_parser.h"
#include "../fcerrors.h"

// Parse program to build the symbol table and check for errors
// Returns the pointer to a newly allocated horizon_program_t
// If there are any errors, they will be added to the err_array, as many as
// err_array_size allows. The horizon_program_t return will have the total error count.
// After running horizon_parse, run horizon_free on the pointer to free its
// allocated memory, regardless of if the program was error-free or not.
horizon_program_t *horizon_parse(FILE *fd, error_t *err_array, int err_array_size)
{
    fseek(fd, 0, SEEK_END);
    int size = ftell(fd);
    rewind(fd);

    horizon_program_t program = { ARCH_HORIZON };
    char *program_buf = malloc(size + 1);
    memset(program_buf, 0, size + 1);

    program.input_buf = program_buf;

    // Read whole program in
    program.len_input = 0;
    fread(program_buf, 1, size, fd);

    // Don't uppercase comments
    int uppercase = 1;
    for (int i = 0; i < size; i++)
    {
        if (program_buf[i] == ';')
            uppercase = 0;
        else if (program_buf[i] == '\n')
            uppercase = 1;
        if (uppercase)
            program_buf[i] = toupper(program_buf[i]);
    }

    // Allocate the needed program space
    int symbol_space = 100;
    program.symbols = malloc(sizeof(symbol_t) * symbol_space);
    program.len_symbols_space = symbol_space;

    int data_space = 100;
    program.data = malloc(sizeof(uint32_t) * data_space);
    program.len_data_space = data_space;

    int code_lines_space = 100;
    program.code_lines = malloc(sizeof(char *) * code_lines_space);
    program.len_code_lines_space = code_lines_space;
    program.code_line_indices = malloc(sizeof(int) * code_lines_space);

    int code_space = 100;
    program.code = malloc(sizeof(int64_t) * code_space);
    program.len_code_space = code_space;

    int macro_space = 100;
    program.macros = malloc(sizeof(horizon_macro_t) * macro_space);
    program.len_macros_space = macro_space;

    // Account for the initial start instruction
    program.data_offset = 1;

    program.name = NULL;
    program.desc = NULL;

    int retval = NO_ERR;

    ho_add_builtin_macros(&program);

    int line = 1;
    while (retval != ERR_EOF)
    {
        // This will consume whitespace even without newlines
        while (!(retval = ho_match_newline(&program_buf)))
            line++;
        if (retval == ERR_EOF)
            break;

        int lines_consumed = 0;
        program.curr_line = line;
        retval = ho_parse_statement(&program, &lines_consumed, &program_buf);
        if (retval != NO_ERR)
        {
            program.error_count++;
            ho_parser_perror(NULL, retval, line);
            retval = ho_match_error(&program_buf);
        } else
        {
            // successfully parsed lines should end here
            retval = ho_match_newline(&program_buf);
            if (retval == ERR_NO_MATCH)
            {
                program.error_count++;
                ho_parser_perror(NULL, ERR_TOO_MANY_ARGUMENTS, line);
                retval = ho_match_error(&program_buf);
            }
        }

        line += lines_consumed;
        line++;
    }

    // printf("Instructions:\n");
    char jmp_start_instr[HORIZON_IDENT_MAX_LEN + 1] = { 0 };
    sprintf(jmp_start_instr, "JMP #%d\n", program.code_start + 1 + program.len_data);
    ho_parse_instruction(&program, jmp_start_instr);

    program.len_code_space += program.len_data;
    program.code = realloc(program.code, sizeof(int64_t) * program.len_code_space);

    for (int i = 0; i < program.len_data; i++)
    {
        program.code[program.len_code] = program.data[i];
        program.len_code++;
    }
    for (int i = 0; i < program.len_code_lines; i++)
    {
        int res = ho_parse_instruction(&program, program.code_lines[i]);
        if (!(res == NO_ERR || res == ERR_EOF))
        {
            ho_parser_perror(NULL, res, program.code_line_indices[i]);
            program.error_count++;
        }
    }

    // Debug: output program binary
    if (DEBUG)
    {
        for (int i = 0; i < program.len_code; i++)
        {
            printf("%08X  %d\n", (uint32_t) (program.code[i] & 0xFFFFFFFF), (int) program.code[i]);
        }
    }

    horizon_program_t *ret = malloc(sizeof(horizon_program_t));
    *ret = program;
    return ret;
}

// Frees memory allocated by horizon_parse
void horizon_free(horizon_program_t *program)
{
    ho_init_regex(1);

    if (!program)
        return;
    if (program->input_buf)
        free(program->input_buf);
    if (program->len_symbols)
    {
        for (int i = 0; i < program->len_symbols; i++)
            free(program->symbols[i].name);
    }
    if (program->symbols)
        free(program->symbols);
    if (program->data)
        free(program->data);
    if (program->code_lines)
        free(program->code_lines);
    if (program->code_line_indices)
        free(program->code_line_indices);
    if (program->desc)
        free(program->desc);
    if (program->code)
        free(program->code);
    for (int i = 0; i < program->len_macros; i++)
    {
        for (int j = 0; j < program->macros[i].len; j++)
            free(program->macros[i].lines[j]);
        free(program->macros[i].lines);
    }
    if (program->macros)
        free(program->macros);
    free(program);
    return;
}

// Parse the program at the given src_filepath and save the result in the given
// destination path dst_filename.
// The options struct changes the way the procedure operates
void horizon_compile(const char *dst_filename, const char *src_filename, struct horizon_compiler_opt *options)
{
    printf("not implemented\n");
    return;
}
