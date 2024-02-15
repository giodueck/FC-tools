#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include "overture.h"
#include "program.h"

static overture_register_map_t registers = { 0 };
static program_t program = { 0 };
static char *program_buf = NULL;

enum overture_line_type { OV_EMPTY, OV_LABEL, OV_IMM, OV_REG, OV_INS, OV_LABEL_IMM };

static regex_t label            = { 0 };
static regex_t immediate        = { 0 };
static regex_t reg              = { 0 };
static regex_t instr            = { 0 };
static regex_t label_immediate  = { 0 };

static int code_len = 0;
static uint8_t *code = NULL;


// Note: This function returns a pointer to a substring of the original string.
// If the given string was allocated dynamically, the caller must not overwrite
// that pointer with the returned value, since the original pointer must be
// deallocated using the same allocator with which it was allocated.  The return
// value must NOT be deallocated using free() etc.
char *trimwhitespace(char *str)
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

// Parse program to build the symbol table and check for any errors
// Returns 0 if no errors
// Allocates memory, free with overture_free()
int overture_parse(FILE *fd)
{
    fseek(fd, 0, SEEK_END);
    int size = ftell(fd);
    rewind(fd);

    if (program_buf != NULL)
        overture_free();
    program_buf = malloc(size + 1);
    char *buf = malloc(size + 1);

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
            buf[j++] = c;
            i++;

            // Cut off comments, but keep reading the file
            if (c == ';')
                buf[j - 1] = '\0';
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

    // First Pass: Parse empty lines, labels, immediates, instructions
    int code_space = 1000;
    code = malloc(sizeof(uint8_t) * code_space);
    code_len = 0;

    int symbol_space = 1000;
    program.symbols = malloc(sizeof(symbol_t) * symbol_space);
    program.len_symbols = 0;

    program.line_executable = malloc(sizeof(int) * program.len_lines);
    for (int i = 0; i < program.len_lines; i++)
    {
        if (program.lines[i][0] == '\0')
        {
            program.line_executable[i] = 0;
            continue;
        }
        program.line_executable[i] = 1;

        if (code_len >= code_space)
        {
            code_space += 1000;
            code = realloc(code, sizeof(uint8_t) * code_space);
        }

        regmatch_t match = { 0 };
        int r;
        if ((r = regexec(&immediate, program.lines[i], 0, &match, 0)) == 0)
        {
            // Copy value directly as code
            code[code_len++] = atoi(program.lines[i]);
            continue;
        } else if ((r = regexec(&label, program.lines[i], 0, &match, 0)) == 0)
        {
            if (program.len_symbols >= symbol_space)
            {
                symbol_space += 1000;
                program.symbols = realloc(program.symbols, sizeof(symbol_t) * symbol_space);
            }

            // Copy symbol to symbols array
            program.symbols[program.len_symbols].name = malloc(sizeof(char) * (strlen(program.lines[i]) + 1));

            program.symbols[program.len_symbols].value = code_len;

            strcpy(program.symbols[program.len_symbols].name, program.lines[i]);
            program.symbols[program.len_symbols].name[strlen(program.lines[i]) - 1] = '\0';
            program.len_symbols++;

            continue;
        } else
        {
            // TODO: instructions
            code[code_len++] = 0;
            continue;
        }
    }

    printf("## First Pass ##\n");
    printf("Code size: %d\n", code_len);
    for (int i = 0; i < code_len; i++)
    {
        printf("%hhu ", code[i]);
    }
    printf("\n");
    for (int i = 0; i < program.len_symbols; i++)
    {
        printf("%s: %u\n", program.symbols[i].name, program.symbols[i].value);
    }
    printf("\n");

    // Second pass: replace labels with immediates
    // TODO:

    return 0;
}

// Frees up memory allocated during parsing
void overture_free()
{
    free(program_buf);
    program_buf = NULL;

    for (int i = 0; i < program.len_symbols; i++)
        free(program.symbols[i].name);
    free(program.symbols);
    free(program.lines);
    free(program.line_executable);
    program = (program_t) { 0, NULL, 0, NULL, NULL };

    free(code);
}

// Interpret and execute an instruction
int overture_execute(const char *instr)
{

    return 0;
}

// Returns the current state of the registers
overture_register_map_t overture_get_registers();

// Sets the registers, useful for debugging
void overture_set_registers(overture_register_map_t reg_map);

// Prints all executable lines
void overture_print()
{
    if (program.lines != NULL)
    {
        for (int i = 0; i < program.len_lines; i++)
        {
            if (program.line_executable[i])
                printf("%s\n", program.lines[i]);
        }
    }
}

// Compiles the needed regular expressions
void overture_init()
{
    regcomp(&label, "^[a-zA-Z_][a-zA-Z0-9_]*:$", REG_NOSUB);
    regcomp(&immediate, "^\\([0-5]\\{0,1\\}[0-9]\\|6[0-3]\\)$", REG_NOSUB);
    regcomp(&reg, "[rR][0-7]", 0);
}

// Frees the used regular expressions
void overture_quit()
{
    regfree(&label);
    regfree(&immediate);
    regfree(&reg);
}
