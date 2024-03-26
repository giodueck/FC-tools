#include <limits.h>
#include <regex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "horizon_parser.h"
#include "../fcerrors.h"

struct horizon_regex_t {
    regex_t literal_re;
    regex_t register_re;
    regex_t identifier_re;
    regex_t instruction_re;
};

static struct horizon_regex_t horizon_regex = { 0 };

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
    "NIL"
};

const char *horizon_reserved_ident[] = {
    "RAM"
};

enum horizon_opcode {
    ADD =       0,
    SUB =       1,
    MUL =       2,
    DIV =       3,
    MOD =       4,
    EXP =       5,
    LSH =       6,
    RSH =       7,
    AND =       8,
    OR =        9,
    NOT =       10,
    XOR =       11,
    BCAT =      12,
    HCAT =      13,
    ADDS =      16,
    SUBS =      17,
    MULS =      18,
    DIVS =      19,
    MODS =      20,
    EXPS =      21,
    LSHS =      22,
    RSHS =      23,
    ANDS =      24,
    ORS =       25,
    NOTS =      26,
    XORS =      27,
    BCATS =     28,
    HCATS =     29,
    JEQ =       32,
    JNE =       33,
    JLT =       34,
    JGT =       35,
    JLE =       36,
    JGE =       37,
    JNG =       38,
    JPZ =       39,
    JVS =       40,
    JVC =       41,
    JMP =       42,
    NOOP =      43,
    STORE =     48,
    LOAD =      49,
    STOREI =    50,
    LOADI =     51,
    STORED =    52,
    LOADD =     53,
    PUSH =      56,
    POP =       57
};

// Initilizes all regex used by the parser
int ho_init_regex()
{
    static int reret = INT_MAX;
    if (reret == INT_MAX)
    {
        // Match only the literal, which must be at the beginning of the string
        reret = regcomp(&horizon_regex.literal_re, "^[\\(0x[:xdigit:]\\{1,\\}\\)\\(-\\{0,1\\}[:digit:]\\{1,\\}\\)]", REG_ICASE);
        if (reret)
        {
            char errbuf[BUFSIZ] = "";
            regerror(reret, &horizon_regex.literal_re, errbuf, BUFSIZ);
            fprintf(stderr, "match_literal: %s\n", errbuf);
            exit(1);
        }

        // Match only the register, which must be at the beginning of the string
        reret = regcomp(&horizon_regex.register_re, "^\\(R[0-9]\\)\\|\\(R1[01]\\)\\|\\(AR\\)\\|\\(SP\\)\\|\\(LR\\)\\|\\(PC\\)\\|\\(NIL\\)", REG_ICASE);
        if (reret)
        {
            char errbuf[BUFSIZ] = "";
            regerror(reret, &horizon_regex.register_re, errbuf, BUFSIZ);
            fprintf(stderr, "match_register: %s\n", errbuf);
            exit(1);
        }

        // Match only the identifier, which must be at the beginning of the string
        reret = regcomp(&horizon_regex.identifier_re, "^[A-Z_][A-Z_0-9]*", REG_ICASE);
        if (reret)
        {
            char errbuf[BUFSIZ] = "";
            regerror(reret, &horizon_regex.identifier_re, errbuf, BUFSIZ);
            fprintf(stderr, "match_register: %s\n", errbuf);
            exit(1);
        }

        // Match only the instruction, which must be at the beginning of the string
        reret = regcomp(&horizon_regex.instruction_re, "^[A-Z]*", REG_ICASE);
        if (reret)
        {
            char errbuf[BUFSIZ] = "";
            regerror(reret, &horizon_regex.instruction_re, errbuf, BUFSIZ);
            fprintf(stderr, "match_register: %s\n", errbuf);
            exit(1);
        }
    }
    else
    {
        return reret;
    }

    return NO_ERR;
}

// Match a base 8, 10 or 16 number and set dest to its value
int ho_match_literal(uint32_t *dest, char **buf)
{
    static regex_t regex;
    static int reret = INT_MAX;
    if (reret == INT_MAX)
    {
        reret = ho_init_regex();
        regex = horizon_regex.literal_re;
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
int ho_match_imm8(uint32_t *dest, char **buf)
{
    if (**buf == '#')
        (*buf)++;
    else
    {
        *dest = -1;
        return ERR_NO_MATCH;
    }

    int res = ho_match_literal(dest, buf);
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
int ho_match_imm16(uint32_t *dest, char **buf)
{
    if (**buf == '#')
        (*buf)++;
    else
    {
        *dest = -1;
        return ERR_NO_MATCH;
    }

    int res = ho_match_literal(dest, buf);
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
int ho_match_register(uint32_t *dest, char **buf)
{
    static regex_t regex;
    static int reret = INT_MAX;
    if (reret == INT_MAX)
    {
        reret = ho_init_regex();
        regex = horizon_regex.register_re;
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

    // Advance buffer
    *buf += match.rm_eo - match.rm_so;
    return NO_ERR;
}

// Match an identifier and set dest to its length
// Whether the identifier is new or already defined can be handled by the parser rule
// The buffer is not advanced by this function, instead the caller can read dest
// bytes and obtain the identifier, then advance the buffer dest positions
int ho_match_identifier(uint32_t *dest, char **buf)
{
    static regex_t regex;
    static int reret = INT_MAX;
    if (reret == INT_MAX)
    {
        reret = ho_init_regex();
        regex = horizon_regex.identifier_re;
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

    if (ho_is_reserved(word))
        return ERR_RESERVED_WORD;

    *dest = len;
    return NO_ERR;
}

// Match whitespace, excluding newlines. Never returns errors, even with no whitespace
int ho_match_whitespace(char **buf)
{
    while (**buf == ' ' || **buf == '\t')
        (*buf)++;
    return NO_ERR;
}

// Match a single newline character, preceded and followed optionally by whitespace
// and optionally preceded by a comment
int ho_match_newline(char **buf)
{
    ho_match_comment(buf);
    if (**buf == '\n')
    {
        (*buf)++;
        if (**buf == '\0')
            return ERR_EOF;
    } else if (**buf == '\0')
        return ERR_EOF;
    else
        return ERR_NO_MATCH;
    ho_match_whitespace(buf);

    return NO_ERR;
}

// Match a comment without its ending newline
int ho_match_comment(char **buf)
{
    ho_match_whitespace(buf);
    if (**buf == ';')
    {
        while (**buf != '\n')
            (*buf)++;
    }
    return NO_ERR;
}

// Match anything until the next newline, then call match_newline
int ho_match_error(char **buf)
{
    while (**buf != '\n' && **buf != '\0')
        (*buf)++;
    return ho_match_newline(buf);
}


// Match the noop instruction and set dest to the opcode
int ho_match_noop(uint32_t *dest, char **buf)
{
    static regex_t regex;
    static int reret = INT_MAX;
    if (reret == INT_MAX)
    {
        reret = ho_init_regex();
        regex = horizon_regex.instruction_re;
    }

    regmatch_t match;
    int ret = regexec(&regex, *buf, 1, &match, 0);

    int len = match.rm_eo - match.rm_so;
    if (ret != 0 || len != 4)
        return ERR_NO_MATCH;

    if (strncmp(*buf, "NOOP", len) == 0)
    {
        *dest = NOOP;
        *buf += len;
        return NO_ERR;
    }

    return ERR_NO_MATCH;
}

//Match the not or the pop instruction and set dest to the opcode
int ho_match_not_pop(uint32_t *dest, char **buf)
{
    static regex_t regex;
    static int reret = INT_MAX;
    if (reret == INT_MAX)
    {
        reret = ho_init_regex();
        regex = horizon_regex.instruction_re;
    }

    regmatch_t match;
    int ret = regexec(&regex, *buf, 1, &match, 0);

    int len = match.rm_eo - match.rm_so;
    if (ret != 0 || len < 3 || len > 4)
        return ERR_NO_MATCH;

    if (len == 3)
    {
        if (strncmp(*buf, "NOT", len) == 0)
        {
            *dest = NOT;
            *buf += len;
            return NO_ERR;
        } else if (strncmp(*buf, "POP", len) == 0)
        {
            *dest = POP;
            *buf += len;
            return NO_ERR;
        }
    } else
    {
        if (strncmp(*buf, "NOTS", len) == 0)
        {
            *dest = NOTS;
            *buf += len;
            return NO_ERR;
        }
    }

    return ERR_NO_MATCH;
}

// Match any alu instruction except not and set dest to the opcode
int ho_match_alu(uint32_t *dest, char **buf)
{
    static regex_t regex;
    static int reret = INT_MAX;
    if (reret == INT_MAX)
    {
        reret = ho_init_regex();
        regex = horizon_regex.instruction_re;
    }

    regmatch_t match;
    int ret = regexec(&regex, *buf, 1, &match, 0);

    int len = match.rm_eo - match.rm_so;
    if (ret != 0 || len < 2 || len > 5)
        return ERR_NO_MATCH;

    if (len == 2 && strncmp(*buf, "OR", len) == 0)
    {
        *dest = OR;
        *buf += len;
        return NO_ERR;
    } else if (len == 3)
    {
        if (strncmp(*buf, "ADD", len) == 0)
        {
            *dest = ADD;
            *buf += len;
            return NO_ERR;
        }
        // all the other ones, i need to eat
    }

    return ERR_NO_MATCH;
}

// Match the push instruction and set dest to the opcode
int ho_match_push(uint32_t *dest, char **buf)
{
    static regex_t regex;
    static int reret = INT_MAX;
    if (reret == INT_MAX)
    {
        reret = ho_init_regex();
        regex = horizon_regex.instruction_re;
    }

    regmatch_t match;
    int ret = regexec(&regex, *buf, 1, &match, 0);

    int len = match.rm_eo - match.rm_so;
    if (ret != 0 || len != 4)
        return ERR_NO_MATCH;

    if (strncmp(*buf, "PUSH", len) == 0)
    {
        *dest = PUSH;
        *buf += len;
        return NO_ERR;
    }

    return ERR_NO_MATCH;
}

// Match any jump instruction and set dest to the opcode
int ho_match_cond(uint32_t *dest, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

// Match any memory access instruction and set dest to the opcode
int ho_match_mem(uint32_t *dest, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}


int ho_parse_value(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int ho_parse_value_list(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int ho_parse_directive(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int ho_parse_label(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int ho_parse_format_2(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int ho_parse_format_3(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int ho_parse_format_4(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int ho_parse_format_5(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int ho_parse_alu(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int ho_parse_ram(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int ho_parse_cond(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int ho_parse_push(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int ho_parse_instruction(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int ho_parse_statement(program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}


// Print an error message for a parser error
void ho_parser_perror(char *msg, int error)
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
int ho_is_reserved(const char *word)
{
    for (int i = 0; i < sizeof(horizon_reserved)/sizeof(char*); i++)
    {
        if (strcmp(word, horizon_reserved[i]) == 0)
            return 1;
    }
    return 0;
}

// Returns 1 if the word string is a reserved identifier, and 0 if not
int ho_is_reserved_ident(const char *ident)
{
    for (int i = 0; i < sizeof(horizon_reserved_ident)/sizeof(char*); i++)
    {
        if (strcmp(ident, horizon_reserved_ident[i]) == 0)
            return 1;
    }
    return 0;
}
