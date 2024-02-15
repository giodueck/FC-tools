#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include "overture.h"
#include "program.h"

static program_t program = { 0 };
static char *program_buf = NULL;

static regex_t label            = { 0 };
static regex_t immediate        = { 0 };

enum machine_code_def {
ADD = 64,
SUB = 65,
MUL = 66,
DIV = 67,
AND = 68,
ORR = 69,
NOT = 70,
XOR = 71,
NOP = 192,
JEQ = 193,
JLT = 194,
JLE = 195,
JMP = 196,
JNE = 197,
JGE = 198,
JGT = 199,
};

static int code_len = 0;
static int *code = NULL;
static int error_count = 0;

static overture_register_map_t reg = { 0 };
static int control_at = -1;
static int cycle = -1;
static int end_instruction = -1;
static int *breakpoints = NULL;

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
    code = malloc(sizeof(int) * code_space);
    code_len = 0;

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

            // Labels are not executed
            program.line_executable[i] = 0;

            continue;
        } else
        {
            code[code_len++] = -1;
            continue;
        }
    }

    #ifdef DEBUG
    printf("## First Pass ##\n");
    printf("Code size: %d\n", code_len);
    for (int i = 0; i < code_len; i++)
    {
        printf("%d ", code[i]);
    }
    printf("\n");
    for (int i = 0; i < program.len_symbols; i++)
    {
        printf("%s: %u\n", program.symbols[i].name, program.symbols[i].value);
    }
    printf("\n");
    #endif

    // Second pass: replace labels with immediates, instructions
    i = 0;
    for (int ci = 0; ci < code_len; ci++, i++)
    {
        while (!program.line_executable[i])
            i++;

        for (int j = 0; j < program.len_symbols; j++)
        {
            if (strcmp(program.symbols[j].name, program.lines[i]) == 0)
            {
                code[ci] = program.symbols[j].value;
                break;
            }
        }

        if (code[ci] < 0)
        {
            int op = parse_instruction(program.lines[i]);
            if (op < 0)
            {
                error_count++;
                printf("Error on line %d: %s\n", i, program.lines[i]);
                continue;
            }
            code[ci] = op;
        }
    }

    #ifdef DEBUG
    printf("## Second Pass ##\n");
    for (int i = 0; i < code_len; i++)
    {
        printf("%d ", code[i]);
    }
    printf("\n\n");
    #endif

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
    program = (program_t) { 0, NULL, 0, NULL, NULL };

    free(code);
}

// Returns the current state of the registers
overture_register_map_t overture_get_registers()
{
    return reg;
}

// Sets the registers, useful for debugging
void overture_set_registers(overture_register_map_t reg_map)
{
    reg = reg_map;
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

// Used to tell the program where to halt. If not used, the program will run forever
// If a negative value is given, the label "end" will be set as the end if it exists
// On success, returns the code word address for the given line, otherwise returns -1
int overture_set_end(int line)
{
    end_instruction = -1;

    // Look for the label "end"
    if (line < 0)
    {
        for (int i = 0; i < program.len_symbols; i++)
        {
            if (!strcmp(program.symbols[i].name, "end"))
            {
                end_instruction = program.symbols[i].value;
                return end_instruction;
            }
        }
    }

    // If the line if after the last instruction, that's an error
    if (line > program.len_lines)
        return -1;

    // Look for the code word for the given line
    for (int i = 0; i < line; i++)
    {
        end_instruction += program.line_executable[i];
    }
    return end_instruction;
}

// Execute one instruction on the registers
static void overture_execute(int op, int *jump_addr)
{
    if (op < 0 || op > 255)
        return;
    if (op < 64)
    {
        reg.r0 = op;
        return;
    }

    switch (op)
    {
    case ADD:
        reg.r3 = reg.r1 + reg.r2;
        break;
    case SUB:
        reg.r3 = reg.r1 - reg.r2;
        break;
    case MUL:
        reg.r3 = reg.r1 * reg.r2;
        break;
    case DIV:
        if (reg.r2 == 0) reg.r3 = 0;
        else reg.r3 = reg.r1 / reg.r2;
        break;
    case AND:
        reg.r3 = reg.r1 & reg.r2;
        break;
    case ORR:
        reg.r3 = reg.r1 | reg.r2;
        break;
    case NOT:
        reg.r3 = ~reg.r1;
        break;
    case XOR:
        reg.r3 = reg.r1 ^ reg.r2;
        break;
    case NOP:
        break;
    case JEQ:
        if (reg.r3 == 0)
            *jump_addr = reg.r0;
        break;
    case JLT:
        if (reg.r3 < 0)
            *jump_addr = reg.r0;
        break;
    case JLE:
        if (reg.r3 <= 0)
            *jump_addr = reg.r0;
        break;
    case JMP:
        *jump_addr = reg.r0;
        break;
    case JNE:
        if (reg.r3 != 0)
            *jump_addr = reg.r0;
        break;
    case JGE:
        if (reg.r3 >= 0)
            *jump_addr = reg.r0;
        break;
    case JGT:
        if (reg.r3 > 0)
            *jump_addr = reg.r0;
        break;
    default:
        if ((op >> 6) == 2)
        {
            uint32_t rnull = 0;
            uint32_t *regarray[8] = { &reg.r0, &reg.r1, &reg.r2, &reg.r3, &reg.r4, &reg.r5, &rnull, &rnull };

            *regarray[op & 7] = *regarray[(op >> 3) & 7];
        }
        break;
    }
}

// Runs the parsed program if it has no errors
// Command determines how far the program is allowed to run before it stops
int overture_run(int command)
{
    if (error_count != 0)
        return -1;

    switch (command)
    {
        case RUN:
            control_at = 0;
            while (control_at < code_len)
            {
                if (control_at == end_instruction)
                    break;
                int j = -1;
                overture_execute(code[control_at], &j);
                if (j >= 0)
                    control_at = j;
                else
                    control_at++;
                cycle++;
            }
            return 0;
        case STEP:
        case STEP_INTO:
        case CONTINUE:
        default:
            return -1;
    }
}
