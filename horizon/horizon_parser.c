#include <limits.h>
#include <regex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "horizon_parser.h"
#include "../fcerrors.h"

const char *horizon_reserved[] = {
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "MOD",
    "EXP",
    "LSH",
    "RSH",
    "AND",
    "OR",
    "NOT",
    "XOR",
    "BCAT",
    "HCAT",
    "ADDS",
    "SUBS",
    "MULS",
    "DIVS",
    "MODS",
    "EXPS",
    "LSHS",
    "RSHS",
    "ANDS",
    "ORS",
    "NOTS",
    "XORS",
    "BCATS",
    "HCATS",
    "JEQ",
    "JNE",
    "JLT",
    "JGT",
    "JLE",
    "JGE",
    "JNG",
    "JPZ",
    "JVS",
    "JVC",
    "JMP",
    "NOOP",
    "STORE",
    "LOAD",
    "STOREI",
    "LOADI",
    "STORED",
    "LOADD",
    "PUSH",
    "POP",
    "HALT",
    "RESET",
    "CMP",
    "INC",
    "INCS",
    "DEC",
    "DECS",
    "CALL",
    "RETURN",
    "MOV",
    "MOVS",
    "MOV16",
    "CONST",
    "DEFINE",
    "INCLUDE",
    "VAR",
    "ARRAY",
    "R0",
    "R1",
    "R2",
    "R3",
    "R4",
    "R5",
    "R6",
    "R7",
    "R8",
    "R9",
    "R10",
    "R11",
    "AR",
    "SP",
    "LR",
    "PC",
    "NIL",
    "RAM_START"
};

// Match a base 8, 10 or 16 number and set dest to its value
int match_literal(uint32_t *dest, char **buf)
{
    static regex_t regex;
    static int reret = INT_MAX;
    if (reret == INT_MAX)
    {
        // Match only the literal, which must be at the beginning of the string
        reret = regcomp(&regex, "^[\\(0x[:xdigit:]\\{1,\\}\\)\\(-\\{0,1\\}[:digit:]\\{1,\\}\\)]", REG_ICASE);
        if (reret)
        {
            char errbuf[BUFSIZ] = "";
            regerror(reret, &regex, errbuf, BUFSIZ);
            fprintf(stderr, "match_literal: %s\n", errbuf);
            exit(1);
        }
    }

    regmatch_t match;
    int ret = regexec(&regex, *buf, 1, &match, 0);

    if (ret != 0)
        return ERR_NO_MATCH;

    char *endptr = NULL;
    int64_t num = strtol(*buf + match.rm_so, &endptr, 0);

    if (num < INT32_MIN || num > UINT32_MAX)
    {
        *dest = -1;
        return ERR_OUT_OF_RANGE_32;
    }

    // In case it is negative
    *dest = num & 0xFFFFFFFF;

    // Advance buffer pointer
    *buf = endptr;

    return NO_ERR;
}

// Match a literal starting with '#' which is 8-bits wide and set dest to its value
int match_imm8(uint32_t *dest, char **buf)
{
    if (**buf == '#')
        (*buf)++;
    else
    {
        *dest = -1;
        return ERR_NO_MATCH;
    }

    int res = match_literal(dest, buf);
    if (res != NO_ERR)
    {
        switch (res)
        {
        case ERR_OUT_OF_RANGE_32:
            return ERR_OUT_OF_RANGE_8;
        default:
            return res;
        }
    }

    int32_t num = *dest & 0xFFFFFFFF;
    if (num < INT8_MIN || num > UINT8_MAX)
    {
        *dest = -1;
        return ERR_OUT_OF_RANGE_8;
    }
    *dest &= 0xFF;

    return NO_ERR;
}

// Match a literal starting with '#' which is 16-bits wide and set dest to its value
int match_imm16(uint32_t *dest, char **buf)
{
    if (**buf == '#')
        (*buf)++;
    else
    {
        *dest = -1;
        return ERR_NO_MATCH;
    }

    int res = match_literal(dest, buf);
    if (res != NO_ERR)
    {
        switch (res)
        {
        case ERR_OUT_OF_RANGE_32:
            return ERR_OUT_OF_RANGE_16;
        default:
            return res;
        }
    }

    int32_t num = *dest & 0xFFFFFFFF;
    if (num < INT16_MIN || num > UINT16_MAX)
    {
        *dest = -1;
        return ERR_OUT_OF_RANGE_16;
    }
    *dest &= 0xFFFF;

    return NO_ERR;
}

// Match a register and set dest to its number
int match_register(uint32_t *dest, char **buf)
{
    static regex_t regex;
    static int reret = INT_MAX;
    if (reret == INT_MAX)
    {
        // Match only the register, which must be at the beginning of the string
        reret = regcomp(&regex, "^\\(R[0-9]\\)\\|\\(R1[01]\\)\\|\\(AR\\)\\|\\(SP\\)\\|\\(LR\\)\\|\\(PC\\)\\|\\(NIL\\)", REG_ICASE);
        if (reret)
        {
            char errbuf[BUFSIZ] = "";
            regerror(reret, &regex, errbuf, BUFSIZ);
            fprintf(stderr, "match_register: %s\n", errbuf);
            exit(1);
        }
    }

    regmatch_t match;
    int ret = regexec(&regex, *buf, 1, &match, 0);

    if (ret != 0)
        return ERR_NO_MATCH;

    if (match.rm_eo - match.rm_so == 3)
    {
        if ((*buf)[2] == '0')
            *dest = 10;
        else if ((*buf)[2] == '1')
            *dest = 11;
        else // NIL
            *dest = 255;

        *buf += match.rm_eo - match.rm_so;
        return NO_ERR;
    }

    switch ((*buf)[0])
    {
    case 'r':
    case 'R':
        *dest = (*buf)[1] - '0';
        break;
    case 'a':
    case 'A':
        *dest = 12;
        break;
    case 's':
    case 'S':
        *dest = 13;
        break;
    case 'l':
    case 'L':
        *dest = 14;
        break;
    case 'p':
    case 'P':
        *dest = 15;
        break;
    default:
        *dest = -1;
        return ERR_NO_MATCH;
    }

    *buf += match.rm_eo - match.rm_so;
    return NO_ERR;
}

// Match an identifier and set dest to its length
// Whether the identifier is new or already defined can be handled by the parser rule
int match_identifier(uint32_t *dest, char **buf)
{
    static regex_t regex;
    static int reret = INT_MAX;
    if (reret == INT_MAX)
    {
        // Match only the register, which must be at the beginning of the string
        reret = regcomp(&regex, "^[A-Z_][A-Z_0-9]*", REG_ICASE);
        if (reret)
        {
            char errbuf[BUFSIZ] = "";
            regerror(reret, &regex, errbuf, BUFSIZ);
            fprintf(stderr, "match_register: %s\n", errbuf);
            exit(1);
        }
    }

    regmatch_t match;
    int ret = regexec(&regex, *buf, 1, &match, 0);

    *dest = -1;
    if (ret != 0)
        return ERR_NO_MATCH;

    int len = match.rm_eo - match.rm_so;
    if (len > 255)
        return ERR_IDENT_TOO_LONG;

    char word[256] = { 0 };
    strncpy(word, *buf, len);

    if (is_reserved(word))
        return ERR_RESERVED_WORD;

    *dest = len;
    return NO_ERR;
}

// Match whitespace, excluding newlines. Never returns errors, even with no whitespace
int match_whitespace(char **buf)
{
    while (**buf == ' ' || **buf == '\t')
        (*buf)++;
    return NO_ERR;
}

// Match a single newline character, preceded and followed optionally by whitespace
// and optionally preceded by a comment
int match_newline(char **buf)
{
    match_comment(buf);
    if (**buf == '\n')
    {
        (*buf)++;
        if (**buf == '\0')
            return ERR_EOF;
    } else if (**buf == '\0')
        return ERR_EOF;
    else
        return ERR_NO_MATCH;
    match_whitespace(buf);

    return NO_ERR;
}

// Match a comment without its ending newline
int match_comment(char **buf)
{
    match_whitespace(buf);
    if (**buf == ';')
    {
        while (**buf != '\n')
            (*buf)++;
    }
    return ERR_NOT_IMPLEMENTED;
}

// Match anything until the next newline
int match_error(char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}


// Match the noop instruction
int match_noop(uint32_t *dest, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

//Match the not or the pop instruction
int match_not_pop(uint32_t *dest, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

// Match any alu instruction except not
int match_alu(uint32_t *dest, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

// Match the push instruction
int match_push(uint32_t *dest, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

// Match any jump instruction
int match_cond(uint32_t *dest, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

// Match any memory access instruction
int match_mem(uint32_t *dest, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}


int parse_value(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int parse_value_list(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int parse_directive(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int parse_label(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int parse_format_2(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int parse_format_3(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int parse_format_4(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int parse_format_5(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int parse_alu(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int parse_ram(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int parse_cond(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int parse_push(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int parse_instruction(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int parse_statement(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}


// Print an error message for a parser error
void parser_perror(char *msg, int error)
{
    switch (error)
    {
    case NO_ERR:
        break;
    case ERR_NO_MATCH:
        printf("%s: no match found\n", msg);
        break;
    case ERR_OUT_OF_RANGE_8:
        printf("%s: number out of range (less than %d or greater than %u)\n", msg, INT8_MIN, UINT8_MAX);
        break;
    case ERR_OUT_OF_RANGE_16:
        printf("%s: number out of range (less than %d or greater than %u)\n", msg, INT16_MIN, UINT16_MAX);
        break;
    case ERR_OUT_OF_RANGE_32:
        printf("%s: number out of range (less than %d or greater than %u)\n", msg, INT32_MIN, UINT32_MAX);
        break;
    case ERR_UNEXPECTED_NL:
        printf("%s: unexpected newline\n", msg);
        break;
    case ERR_EOF:
        printf("%s: found EOF\n", msg);
        break;
    case ERR_IDENT_TOO_LONG:
        printf("%s: identifier too long (max 255 characters)\n", msg);
        break;
    case ERR_RESERVED_WORD:
        printf("%s: cannot use reserved word as identifier\n", msg);
        break;
    }
}

// Returns 1 if the word string is a reserved word, and 0 if not
int is_reserved(const char *word)
{
    for (int i = 0; i < sizeof(horizon_reserved)/sizeof(char*); i++)
    {
        if (strcmp(word, horizon_reserved[i]) == 0)
            return 1;
    }
    return 0;
}
