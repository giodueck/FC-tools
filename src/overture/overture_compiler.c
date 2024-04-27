#include <ctype.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "overture_compiler.h"
#include "../program.h"

static overture_program_t program = { 0 };
static char *program_buf = NULL;

static regex_t label            = { 0 };
static regex_t immediate        = { 0 };

static int error_count = 0;


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

// Parse an instruction and give back the instruction byte.
// If a parsing error occurs, the return will be negative
static int parse_instruction(const char *ins)
{
    int op = 0;
    int inslen = strlen(ins);
    char line[BUFSIZ];
    for (int i = 0; i < inslen; i++)
    {
        line[i] = tolower(ins[i]);
    }
    line[inslen] = '\0';

    // First token is instruction
    char *tok = strtok(line, " \t");

    if (strlen(tok) != 3)
        return -1;

    // Only mov takes arguments
    if (!strcmp(tok, "mov"))
    {
        int rs, rd;
        tok = strtok(NULL, " \t");
        if (tok[0] != 'r')
            return -1;
        rs = tok[1] - '0';
        tok = strtok(NULL, " \t");
        if (tok[0] != 'r')
            return -1;
        rd = tok[1] - '0';

        if (rs < 0 || rs > 7 || rd < 0 || rs > 7)
            return -1;

        return (2 << 6) | (rs << 3) | rd;
    }

    switch (tok[0])
    {
        case 'a':
            // add, and
            if (!strcmp(tok, "add"))
                return ADD;
            if (!strcmp(tok, "and"))
                return AND;
            return -1;
        case 'd':
            if (!strcmp(tok, "div"))
                return DIV;
            return -1;
        case 'j':
            if (!strcmp(tok, "jeq"))
                return JEQ;
            if (!strcmp(tok, "jlt"))
                return JLT;
            if (!strcmp(tok, "jle"))
                return JLE;
            if (!strcmp(tok, "jmp"))
                return JMP;
            if (!strcmp(tok, "jne"))
                return JNE;
            if (!strcmp(tok, "jge"))
                return JGE;
            if (!strcmp(tok, "jgt"))
                return JGT;
            return -1;
        case 'm':
            if (!strcmp(tok, "mul"))
                return MUL;
            return -1;
        case 'n':
            if (!strcmp(tok, "nop"))
                return NOP;
            if (!strcmp(tok, "not"))
                return NOT;
            return -1;
        case 'o':
            if (!strcmp(tok, "orr"))
                return ORR;
            return -1;
        case 's':
            if (!strcmp(tok, "sub"))
                return SUB;
            return -1;
        case 'x':
            if (!strcmp(tok, "xor"))
                return XOR;
            return -1;

        default:
            return -1;
    }

    return -1;
}

// Parse program to build the symbol table and check for any errors
// Returns 0 if no errors
// Call overture_init() before this function
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

    // First Pass: Parse empty lines, labels, immediates
    int code_space = 1000;
    program.code = malloc(sizeof(int) * code_space);
    program.len_code = 0;

    error_count = 0;

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

        if (program.len_code >= code_space)
        {
            code_space += 1000;
            program.code = realloc(program.code, sizeof(uint8_t) * code_space);
        }

        regmatch_t match = { 0 };
        int r;
        if ((r = regexec(&immediate, program.lines[i], 0, &match, 0)) == 0)
        {
            // Copy value directly as code
            program.code[program.len_code++] = atoi(program.lines[i]);
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

            program.symbols[program.len_symbols].value = program.len_code;

            strcpy(program.symbols[program.len_symbols].name, program.lines[i]);
            program.symbols[program.len_symbols].name[strlen(program.lines[i]) - 1] = '\0';
            program.len_symbols++;

            // Labels are not executed
            program.line_executable[i] = 0;

            continue;
        } else
        {
            program.code[program.len_code++] = -1;
            continue;
        }
    }

    // Second pass: replace labels with immediates, instructions
    i = 0;
    for (int ci = 0; ci < program.len_code; ci++, i++)
    {
        while (!program.line_executable[i])
            i++;

        for (int j = 0; j < program.len_symbols; j++)
        {
            if (strcmp(program.symbols[j].name, program.lines[i]) == 0)
            {
                program.code[ci] = program.symbols[j].value;
                break;
            }
        }

        if (program.code[ci] < 0)
        {
            int op = parse_instruction(program.lines[i]);
            if (op < 0)
            {
                error_count++;
                printf("Error on line %d: %s\n", i, program.lines[i]);
                continue;
            }
            program.code[ci] = op;
        }
    }

    return !(error_count == 0);
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
    program = (overture_program_t) { 0 };

    free(program.code);
}

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
}

// Frees the used regular expressions
void overture_quit()
{
    regfree(&label);
    regfree(&immediate);
}

// Returns the parsed program structure
// Note: this is still malloced memory, and calling overture_free will free it.
// Make sure not to use the program after it has been freed
overture_program_t overture_get_program()
{
    return program;
}
