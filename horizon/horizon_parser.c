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
    "CONST",
    "DEFINE",
    "INCLUDE",
    "VAR",
    "ARRAY",
    "NAME",
    "DESC",
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

// const char *horizon_reserved_ident[] = {
//     "RAM"
// };

// Print an error message
void ho_syntax_error(const char *message, int line_minus_one)
{
    printf("Error on line %d:\n\t%s\n", line_minus_one + 1, message);
}

// Check if an identifier has already been defined
int ho_symbol_exists(horizon_program_t program, const char *token)
{
    for (int i = 0; i < program.len_symbols; i++)
    {
        if (strcmp(token, program.symbols[i].name) == 0)
            return 1;
    }
    return 0;
}

int ho_get_symbol(horizon_program_t program, symbol_t *dest, const char *token)
{
    for (int i = 0; i < program.len_symbols; i++)
    {
        if (strcmp(token, program.symbols[i].name) == 0)
        {
            *dest = program.symbols[i];
            return 1;
        }
    }
    return 0;
}

// Define a new symbol
int ho_add_symbol(horizon_program_t *program, const char *ident, uint32_t value, int type)
{
    int i = program->len_symbols;
    int ident_len = strlen(ident);

    if (program->len_symbols_space <= i)
    {
        program->len_symbols_space += 100;
        program->symbols = realloc(program->symbols, sizeof(symbol_t) * program->len_symbols_space);
    }

    program->symbols[i].name = malloc(HORIZON_IDENT_MAX_LEN + 1);
    strncpy(program->symbols[i].name, ident, ident_len);

    if (ident_len < HORIZON_IDENT_MAX_LEN + 1)
        program->symbols[i].name[ident_len] = '\0';
    else
        program->symbols[i].name[HORIZON_IDENT_MAX_LEN] = '\0';

    program->symbols[i].value = value;
    program->symbols[i].type = type;

    program->len_symbols++;

    return 0;
}

// Define a new data point in program memory
int ho_add_data(horizon_program_t *program, uint32_t value)
{
    if (program->len_data >= program->len_data_space)
    {
        program->len_data_space += 100;
        program->data = realloc(program->data, sizeof(uint32_t) * program->len_data_space);
    }

    program->data[program->len_data++] = value;

    return 0;
}

// Add some predefined macros
int ho_add_builtin_macros(horizon_program_t *program)
{
    struct horizon_builtin_macro_t {
        char *name;
        int argc;
        char *macro_text;
    };

    struct horizon_builtin_macro_t horizon_builtin_macros[] = {
        { .name = "HALT",   .argc = 0, .macro_text = ". JMP PC\n" },
        { .name = "RESET",  .argc = 0, .macro_text = ". XOR SP SP\n. JMP #0\n" },
        { .name = "CMP",    .argc = 2, .macro_text = ". SUBS NIL $1 $2\n" },
        { .name = "INC",    .argc = 1, .macro_text = ". ADD $1 #1\n" },
        { .name = "INCS",   .argc = 1, .macro_text = ". ADDS $1 #1\n" },
        { .name = "DEC",    .argc = 1, .macro_text = ". SUB $1 #1\n" },
        { .name = "DECS",   .argc = 1, .macro_text = ". SUBS $1 #1\n" },
        { .name = "CALL",   .argc = 1, .macro_text = ". ADD LR PC #2\n. JMP $1\n" },
        { .name = "RETURN", .argc = 0, .macro_text = ". JMP LR\n" },
        { .name = "MOV",    .argc = 2, .macro_text = ". ADD $1 NIL $2\n" },
        { .name = "MOVS",   .argc = 2, .macro_text = ". ADDS $1 NIL $2\n" },
        { .name = "MOV16",  .argc = 2, .macro_text = ". PUSH $2\n. POP $1\n" },
    };

    for (int i = 0; i < sizeof(horizon_builtin_macros) / sizeof(struct horizon_builtin_macro_t); i++)
    {
        char *text = horizon_builtin_macros[i].macro_text;
        ho_parse_macro(program, horizon_builtin_macros[i].name, horizon_builtin_macros[i].argc, &text);
    }

    return 0;
}

// Add a line to be parsed in the second parsing pass
int ho_add_code_line(horizon_program_t *program, char *buf)
{
    if (program->len_code_lines >= program->len_code_lines_space)
    {
        program->len_code_lines_space += 100;
        program->code_lines = realloc(program->code_lines, sizeof(uint32_t) * program->len_code_lines_space);
    }

    program->code_lines[program->len_code_lines] = buf;
    program->len_code_lines++;

    return 0;
}

// Add a machine instruction
int ho_add_code(horizon_program_t *program, int64_t code)
{
    if (program->len_code >= program->len_code_space)
    {
        program->len_code_space += 100;
        program->code = realloc(program->code, sizeof(int64_t) * program->len_code_space);
    }

    program->code[program->len_code] = code;
    program->len_code++;

    return 0;
}

// Initilizes all regex used by the parser
int ho_init_regex()
{
    static int reret = INT_MAX;
    if (reret == INT_MAX)
    {
        // Match only the literal, which must be at the beginning of the string
        reret = regcomp(&horizon_regex.literal_re, "^[-\\{0,1\\}\\(0x[:xdigit:]\\{1,\\}\\)\\([:digit:]\\{1,\\}\\)]", REG_ICASE);
        if (reret)
        {
            char errbuf[BUFSIZ] = "";
            regerror(reret, &horizon_regex.literal_re, errbuf, BUFSIZ);
            fprintf(stderr, "ho_init_regex: %s\n", errbuf);
            exit(1);
        }

        // Match only the register, which must be at the beginning of the string
        reret = regcomp(&horizon_regex.register_re, "^\\(R[0-9]\\)\\|\\(R1[01]\\)\\|\\(AR\\)\\|\\(SP\\)\\|\\(LR\\)\\|\\(PC\\)\\|\\(NIL\\)", REG_ICASE);
        if (reret)
        {
            char errbuf[BUFSIZ] = "";
            regerror(reret, &horizon_regex.register_re, errbuf, BUFSIZ);
            fprintf(stderr, "ho_init_regex: %s\n", errbuf);
            exit(1);
        }

        // Match only the identifier, which must be at the beginning of the string
        reret = regcomp(&horizon_regex.identifier_re, "^[A-Z_][A-Z_0-9]*", REG_ICASE);
        if (reret)
        {
            char errbuf[BUFSIZ] = "";
            regerror(reret, &horizon_regex.identifier_re, errbuf, BUFSIZ);
            fprintf(stderr, "ho_init_regex: %s\n", errbuf);
            exit(1);
        }

        // Match only the instruction, which must be at the beginning of the string
        reret = regcomp(&horizon_regex.instruction_re, "^[A-Z]*", REG_ICASE);
        if (reret)
        {
            char errbuf[BUFSIZ] = "";
            regerror(reret, &horizon_regex.instruction_re, errbuf, BUFSIZ);
            fprintf(stderr, "ho_init_regex: %s\n", errbuf);
            exit(1);
        }

        // Match only the directive, which must be at the beginning of the string
        reret = regcomp(&horizon_regex.directive_re, "^\\.[A-Z]*", REG_ICASE);
        if (reret)
        {
            char errbuf[BUFSIZ] = "";
            regerror(reret, &horizon_regex.instruction_re, errbuf, BUFSIZ);
            fprintf(stderr, "ho_init_regex: %s\n", errbuf);
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
    if (len > HORIZON_IDENT_MAX_LEN)
        return ERR_IDENT_TOO_LONG;

    char word[HORIZON_IDENT_MAX_LEN + 1] = { 0 };
    strncpy(word, *buf, len);

    if (ho_is_reserved(word))
        return ERR_RESERVED_WORD;

    *dest = len;
    return NO_ERR;
}

// Match every directive and set dest to the matched directive
int ho_match_directive(uint32_t *dest, char **buf)
{
    static regex_t regex;
    static int reret = INT_MAX;
    if (reret == INT_MAX)
    {
        reret = ho_init_regex();
        regex = horizon_regex.directive_re;
    }

    regmatch_t match;
    int ret = regexec(&regex, *buf, 1, &match, 0);

    int len = match.rm_eo - match.rm_so;
    if (ret != 0 || len < 4 || len > 6)
        return ERR_NO_MATCH;

    if (len == 4)
    {
        if (strncmp(*buf, ".VAR", len) == 0)
        {
            *dest = HO_VAR;
            *buf += len;
            return NO_ERR;
        }
    } else if (len == 5)
    {
        if (strncmp(*buf, ".NAME", len) == 0)
        {
            *dest = HO_NAME;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, ".DESC", len) == 0)
        {
            *dest = HO_DESC;
            *buf += len;
            return NO_ERR;
        }
    } else if (len == 6)
    {
        if (strncmp(*buf, ".CONST", len) == 0)
        {
            *dest = HO_CONST;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, ".ARRAY", len) == 0)
        {
            *dest = HO_ARRAY;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, ".START", len) == 0)
        {
            *dest = HO_START;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, ".MACRO", len) == 0)
        {
            *dest = HO_MACRO;
            *buf += len;
            return NO_ERR;
        }
    }

    return ERR_NO_MATCH;
}

// Match whitespace, excluding newlines. Never returns errors, even with no whitespace
int ho_match_whitespace(char **buf)
{
    while (**buf == ' ' || **buf == '\t')
        (*buf)++;
    return NO_ERR;
}

// Match a sequence of characters
int ho_match_string(const char *str, char **buf)
{
    int len = strlen(str);
    if (strncmp(str, *buf, len) == 0)
    {
        *buf += len;
        return NO_ERR;
    } else
        return ERR_NO_MATCH;
}

// Match a single newline character, preceded and followed optionally by whitespace
// and optionally preceded by a comment
int ho_match_newline(char **buf)
{
    ho_match_whitespace(buf);
    ho_match_comment(NULL, buf);
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
// If dest is not null, set it to point to the first character after the ';'
int ho_match_comment(char **dest, char **buf)
{
    if (**buf == ';')
    {
        if (dest)
            *dest = *buf + 1;
        while (**buf != '\n')
            (*buf)++;
    } else
    {
        return ERR_NO_MATCH;
    }
    return NO_ERR;
}

// Match anything until the next newline, then call match_newline
int ho_match_error(char **buf)
{
    while (**buf != '\n' && **buf != '\0')
    {
        (*buf)++;
    }
    return ho_match_newline(buf);
}

// Match anything until the next newline, then return
int ho_match_error_no_nl(char **buf)
{
    while (**buf != '\n' && **buf != '\0')
    {
        (*buf)++;
    }
    return NO_ERR;
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
        *dest = HO_NOOP;
        *buf += len;
        return NO_ERR;
    }

    return ERR_NO_MATCH;
}

//Match the the not instruction and set dest to the opcode
int ho_match_not(uint32_t *dest, char **buf)
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
            *dest = HO_NOT;
            *buf += len;
            return NO_ERR;
        }
    } else
    {
        if (strncmp(*buf, "NOTS", len) == 0)
        {
            *dest = HO_NOTS;
            *buf += len;
            return NO_ERR;
        }
    }

    return ERR_NO_MATCH;
}

//Match the the pop instruction and set dest to the opcode
int ho_match_pop(uint32_t *dest, char **buf)
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
    if (ret != 0 || len < 3)
        return ERR_NO_MATCH;

    if (len == 3)
    {
        if (strncmp(*buf, "POP", len) == 0)
        {
            *dest = HO_POP;
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
        *dest = HO_OR;
        *buf += len;
        return NO_ERR;
    } else if (len == 3)
    {
        if (strncmp(*buf, "ADD", len) == 0)
        {
            *dest = HO_ADD;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "SUB", len) == 0)
        {
            *dest = HO_SUB;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "MUL", len) == 0)
        {
            *dest = HO_MUL;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "DIV", len) == 0)
        {
            *dest = HO_DIV;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "MOD", len) == 0)
        {
            *dest = HO_MOD;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "EXP", len) == 0)
        {
            *dest = HO_EXP;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "LSH", len) == 0)
        {
            *dest = HO_LSH;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "RSH", len) == 0)
        {
            *dest = HO_RSH;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "AND", len) == 0)
        {
            *dest = HO_AND;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "XOR", len) == 0)
        {
            *dest = HO_XOR;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "ORS", len) == 0)
        {
            *dest = HO_ORS;
            *buf += len;
            return NO_ERR;
        }
    } else if (len == 4)
    {
        if (strncmp(*buf, "BCAT", len) == 0)
        {
            *dest = HO_BCAT;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "HCAT", len) == 0)
        {
            *dest = HO_HCAT;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "ADDS", len) == 0)
        {
            *dest = HO_ADDS;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "SUBS", len) == 0)
        {
            *dest = HO_SUBS;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "MULS", len) == 0)
        {
            *dest = HO_MULS;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "DIVS", len) == 0)
        {
            *dest = HO_DIVS;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "MODS", len) == 0)
        {
            *dest = HO_MODS;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "EXPS", len) == 0)
        {
            *dest = HO_EXPS;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "LSHS", len) == 0)
        {
            *dest = HO_LSHS;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "RSHS", len) == 0)
        {
            *dest = HO_RSHS;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "ANDS", len) == 0)
        {
            *dest = HO_ANDS;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "XORS", len) == 0)
        {
            *dest = HO_XORS;
            *buf += len;
            return NO_ERR;
        }
    } else if (len == 5)
    {
        if (strncmp(*buf, "BCATS", len) == 0)
        {
            *dest = HO_BCATS;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "HCATS", len) == 0)
        {
            *dest = HO_HCATS;
            *buf += len;
            return NO_ERR;
        }
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
        *dest = HO_PUSH;
        *buf += len;
        return NO_ERR;
    }

    return ERR_NO_MATCH;
}

// Match any jump instruction and set dest to the opcode
int ho_match_cond(uint32_t *dest, char **buf)
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
    if (ret != 0 || len != 3)
        return ERR_NO_MATCH;

    if (strncmp(*buf, "JMP", len) == 0)
    {
        *dest = HO_JMP;
        *buf += len;
        return NO_ERR;
    }
    if (strncmp(*buf, "JEQ", len) == 0)
    {
        *dest = HO_JEQ;
        *buf += len;
        return NO_ERR;
    }
    if (strncmp(*buf, "JNE", len) == 0)
    {
        *dest = HO_JNE;
        *buf += len;
        return NO_ERR;
    }
    if (strncmp(*buf, "JLT", len) == 0)
    {
        *dest = HO_JLT;
        *buf += len;
        return NO_ERR;
    }
    if (strncmp(*buf, "JGT", len) == 0)
    {
        *dest = HO_JGT;
        *buf += len;
        return NO_ERR;
    }
    if (strncmp(*buf, "JLE", len) == 0)
    {
        *dest = HO_JLE;
        *buf += len;
        return NO_ERR;
    }
    if (strncmp(*buf, "JGE", len) == 0)
    {
        *dest = HO_JGE;
        *buf += len;
        return NO_ERR;
    }
    if (strncmp(*buf, "JNG", len) == 0)
    {
        *dest = HO_JNG;
        *buf += len;
        return NO_ERR;
    }
    if (strncmp(*buf, "JPZ", len) == 0)
    {
        *dest = HO_JPZ;
        *buf += len;
        return NO_ERR;
    }
    if (strncmp(*buf, "JVS", len) == 0)
    {
        *dest = HO_JVS;
        *buf += len;
        return NO_ERR;
    }
    if (strncmp(*buf, "JVC", len) == 0)
    {
        *dest = HO_JVC;
        *buf += len;
        return NO_ERR;
    }

    return ERR_NO_MATCH;
}

// Match any memory access instruction and set dest to the opcode
int ho_match_mem(uint32_t *dest, char **buf)
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
    if (ret != 0 || len < 4 || len > 6)
        return ERR_NO_MATCH;

    if (len == 4 && strncmp(*buf, "LOAD", len) == 0)
    {
        *dest = HO_LOAD;
        *buf += len;
        return NO_ERR;
    } else if (len == 5)
    {
        if (strncmp(*buf, "STORE", len) == 0)
        {
            *dest = HO_STORE;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "LOADI", len) == 0)
        {
            *dest = HO_LOADI;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "LOADD", len) == 0)
        {
            *dest = HO_LOADD;
            *buf += len;
            return NO_ERR;
        }
    } else if (len == 6)
    {
        if (strncmp(*buf, "STOREI", len) == 0)
        {
            *dest = HO_STOREI;
            *buf += len;
            return NO_ERR;
        }
        if (strncmp(*buf, "STORED", len) == 0)
        {
            *dest = HO_STORED;
            *buf += len;
            return NO_ERR;
        }
    }

    return ERR_NO_MATCH;
}

// Parses a value and places it into dest
// Expects the tokens:
//  literal
//  identifiers // which must be a constant
int ho_parse_value(horizon_program_t *program, uint32_t *dest, char **buf)
{
    int res = 0;
    uint32_t val = 0;
    char ident[HORIZON_IDENT_MAX_LEN + 1] = { 0 };
    symbol_t symbol = { 0 };

    // Try reading a literal first
    res = ho_match_literal(&val, buf);
    if (res == ERR_NO_MATCH)
        goto ho_parse_value_try_ident;
    if (res != NO_ERR)
        return res;

    *dest = val;
    return NO_ERR;

ho_parse_value_try_ident:
    // Try reading an identifier (has to be a const)
    res = ho_match_identifier(&val, buf);
    if (res == ERR_NO_MATCH)
        return ERR_EXPECTED_CONST_OR_LITERAL;
    if (res != NO_ERR)
        return res;

    // check if const
    strncpy(ident, *buf, val);
    if (!ho_get_symbol(*program, &symbol, ident) || symbol.type != HO_SYM_CONST)
        return ERR_EXPECTED_CONST_OR_LITERAL;

    *dest = symbol.value;
    *buf += val;    // advance buffer
    return NO_ERR;
}

// Parses a comma-separated list of values, which is at most len elements long, but
// can be shorter. If shorter, the non-specified values are set to 0 as in C. The
// list is set in the dest_list array.
// Expects the tokens:
//  value, ',', value_list
//  value
int ho_parse_value_list(horizon_program_t *program, uint32_t **dest_list, int len, char **buf)
{
    int idx = 0;
    int retval;

    int repeat = 0;
    do
    {
        if (idx >= len)
            return ERR_TOO_MANY_VALUES;
        retval = ho_parse_value(program, &(*dest_list)[idx], buf);
        if (retval != NO_ERR)
            return retval;

        ho_match_whitespace(buf);
        retval = ho_match_string(",", buf);
        repeat = 0;
        if (retval == NO_ERR)
            repeat = 1;
        ho_match_whitespace(buf);

        idx++;
    } while (repeat);

    return NO_ERR;
}

// Parses a directive
// Expects the tokens:
//  .directive_name {directive_args...}
int ho_parse_directive(horizon_program_t *program, int *lines_consumed, char **buf)
{
    int res = 0;
    uint32_t dirnum = HO_DIR_NONE;

    res = ho_match_directive(&dirnum, buf);
    if (res == ERR_NO_MATCH)
    {
        if (*buf[0] == '.')
            return ERR_UNKNOWN_DIRECTIVE;
        return res;
    }

    uint32_t len = 0;
    char ident[HORIZON_IDENT_MAX_LEN + 1] = { 0 };
    uint32_t value = 0;
    switch (dirnum)
    {
        case HO_CONST:
            // read identifier
            ho_match_whitespace(buf);
            res = ho_match_identifier(&len, buf);
            if (res == ERR_NO_MATCH)
                return ERR_EXPECTED_IDENT;
            if (res != NO_ERR)
                return res;

            // check if appropriate
            strncpy(ident, *buf, len);
            if (ho_symbol_exists(*program, ident))
                return ERR_REDEFINED_IDENT;

            // if so, define with the value of literal
            *buf += len;
            ho_match_whitespace(buf);
            res = ho_match_literal(&value, buf);
            if (res == ERR_NO_MATCH)
                return ERR_EXPECTED_LITERAL;
            if (res != NO_ERR)
                return res;
            ho_add_symbol(program, ident, value, HO_SYM_CONST);
            break;
        case HO_VAR:
            // if instructions have already been parsed, this directive is illegal
            if (program->len_code_lines)
                return ERR_ILLEGAL_DATA_DIRECTIVE;

            // read identifier
            ho_match_whitespace(buf);
            res = ho_match_identifier(&len, buf);
            if (res == ERR_NO_MATCH)
                return ERR_EXPECTED_IDENT;
            if (res != NO_ERR)
                return res;

            // check if appropriate
            strncpy(ident, *buf, len);
            if (ho_symbol_exists(*program, ident))
                return ERR_REDEFINED_IDENT;

            // if so, add the value
            *buf += len;
            ho_match_whitespace(buf);
            res = ho_parse_value(program, &value, buf);
            if (res != NO_ERR)
                return res;
            ho_add_data(program, value);
            ho_add_symbol(program, ident, program->len_data - 1 + program->data_offset, HO_SYM_VAR);
            break;
        case HO_ARRAY:
            // if instructions have already been parsed, this directive is illegal
            if (program->len_code_lines)
                return ERR_ILLEGAL_DATA_DIRECTIVE;

            // read identifier
            ho_match_whitespace(buf);
            res = ho_match_identifier(&len, buf);
            if (res == ERR_NO_MATCH)
                return ERR_EXPECTED_IDENT;
            if (res != NO_ERR)
                return res;

            // check if appropriate
            strncpy(ident, *buf, len);
            if (ho_symbol_exists(*program, ident))
                return ERR_REDEFINED_IDENT;

            // if so, add the values
            *buf += len;

            ho_match_whitespace(buf);
            res = ho_match_string("[", buf);
            if (res != NO_ERR)
                return ERR_EXPECTED_OPEN_B;
            ho_match_whitespace(buf);

            uint32_t array_len = 0;
            res = ho_parse_value(program, &array_len, buf);
            if (res != NO_ERR)
                return res;
            if (array_len == 0)
                return ERR_EXPECTED_NON_ZERO;

            ho_match_whitespace(buf);
            res = ho_match_string("]", buf);
            if (res != NO_ERR)
                return ERR_EXPECTED_CLOSE_B;
            ho_match_whitespace(buf);

            uint32_t *array = malloc(sizeof(uint32_t) * array_len);
            for (int i = 0; i < array_len; i++)
            {
                array[i] = 0;
            }
            ho_match_whitespace(buf);
            res = ho_match_string("{", buf);
            if (res != NO_ERR)
            {
                free(array);
                return ERR_EXPECTED_OPEN_CB;
            }
            ho_match_whitespace(buf);

            res = ho_parse_value_list(program, &array, array_len, buf);
            if (res != NO_ERR)
            {
                free(array);
                return res;
            }

            ho_match_whitespace(buf);
            res = ho_match_string("}", buf);
            if (res != NO_ERR)
            {
                free(array);
                return ERR_EXPECTED_CLOSE_CB;
            }
            ho_match_whitespace(buf);

            ho_add_data(program, array[0]);
            ho_add_symbol(program, ident, program->len_data - 1 + program->data_offset, HO_SYM_VAR);
            for (int i = 1; i < array_len; i++)
                ho_add_data(program, array[i]);

            free(array);
            break;
        case HO_START:
            program->code_start = program->len_code_lines;
            break;
        case HO_NAME:
            ho_match_whitespace(buf);
            res = ho_match_comment(&program->name, buf);
            if (res == ERR_NO_MATCH)
                return ERR_EXPECTED_COMMENT;
            ho_match_whitespace(&program->name);
            break;
        case HO_DESC:
            ho_match_whitespace(buf);
            char *descbuf = NULL;
            res = ho_match_comment(&descbuf, buf);
            if (res == ERR_NO_MATCH)
                return ERR_EXPECTED_COMMENT;

            // Collect consecutive comments and create a malloced string to hold them
            int blocksize = BUFSIZ;
            int descsize = blocksize;
            int desci = 0;
            char *desc = malloc(BUFSIZ);

            char *prev_line_end = NULL;
            while (res == NO_ERR)
            {
                // get one line
                // If there is a space in front of the comment, skip it
                if (descbuf[0] == ' ')
                    descbuf++;

                // Copy characters up to and including the ending newline
                while (1)
                {
                    if (desci >= descsize)
                    {
                        descsize += blocksize;
                        desc = realloc(desc, descsize);
                    }

                    desc[desci++] = descbuf[0];
                    if (descbuf[0] == '\n' || descbuf[0] == '\0')
                        break;
                    descbuf++;
                }

                prev_line_end = *buf;
                ho_match_newline(buf);
                res = ho_match_comment(&descbuf, buf);
            }

            // for the line parsing to go correctly, leave the last newline
            *buf = prev_line_end;

            if (desci > 0 && desc[desci - 1] == '\n')
                desc[desci - 1] = '\0';
            else
                desc[desci] = '\0';
            program->desc = desc;
            break;
        case HO_MACRO:
            // read identifier
            ho_match_whitespace(buf);
            res = ho_match_identifier(&len, buf);
            if (res == ERR_NO_MATCH)
                return ERR_EXPECTED_IDENT;
            if (res != NO_ERR)
                return res;

            // check if appropriate
            strncpy(ident, *buf, len);
            if (ho_symbol_exists(*program, ident))
                return ERR_REDEFINED_IDENT;

            // if so, define with the value of literal
            *buf += len;
            ho_match_whitespace(buf);
            res = ho_match_literal(&value, buf);
            // assume no arguments when not given a number
            if (res == ERR_NO_MATCH)
            {
                if (**buf != '\n')
                    return ERR_EXPECTED_LITERAL;

                value = 0;
            }
            else if (res != NO_ERR)
                return res;
            ho_match_newline(buf);
            res = ho_parse_macro(program, ident, value, buf);
            if (res != NO_ERR)
                return res;
            *lines_consumed = program->macros[program->len_macros - 1].len;
            break;
        default:
            return ERR_NOT_IMPLEMENTED;
            break;
    }

    return NO_ERR;
}

// Parses a macro definition
// Expects a series of instructions prefixed with '.' and whitespace
int ho_parse_macro(horizon_program_t *program, char *name, int argc, char **buf)
{
    int line_count = 0;

    horizon_macro_t macro = { 0 };

    strcpy(macro.name, name);
    macro.argc = argc;
    macro.lines = NULL;

    int res = 0;
    char *prev_line_end = NULL;
    while (1)
    {
        // Prefix dot
        res = ho_match_string(". ", buf);
        if (res != NO_ERR)
        {
            res = ho_match_string(".\t", buf);
            if (res != NO_ERR)
                break;
        }
        ho_match_whitespace(buf);

        // Store the lines, they will be parsed in the second pass
        // count the characters until a newline
        int i = 0;
        while ((*buf)[i] != '\n') i++;
        macro.lines = realloc(macro.lines, sizeof(char *) * (line_count + 1));
        char *line = malloc(i + 1);
        memset(line, 0, i + 1);
        strncpy(line, *buf, i);
        macro.lines[line_count] = line;

        *buf += i;
        prev_line_end = *buf;
        ho_match_newline(buf);
        ho_match_whitespace(buf);

        line_count++;
    }

    if (line_count > 0)
    {
        // for the line parsing to go correctly, leave the last newline
        *buf = prev_line_end;

        macro.len = line_count;
        ho_add_symbol(program, name, argc, HO_SYM_MACRO);

        if (program->len_macros >= program->len_macros_space)
        {
            program->len_macros_space += 100;
            program->macros = realloc(program->macros, sizeof(horizon_macro_t) * program->len_macros_space);
        }
        program->macros[program->len_macros++] = macro;
        res = NO_ERR;
    }
    else
    {
        return ERR_EXPECTED_MACRO;
    }

    return res;
}

// Parses a label
// If the line ends with ':', stores the identifier with the number of instructions
// counted so far as the value.
// Expects the tokens:
//  identifier, ':'
int ho_parse_label(horizon_program_t *program, char **buf)
{
    char ident[HORIZON_IDENT_MAX_LEN + 1] = { 0 };

    // if the line ends up not being a label, to reset the buffer
    char *line_start = *buf;

    uint32_t len;
    int res = ho_match_identifier(&len, buf);
    if (res == ERR_NO_MATCH)
        return res;
    if (res == ERR_RESERVED_WORD)
        goto ho_parse_label_not_label;
    if (res != NO_ERR)
        return res;

    strncpy(ident, *buf, len);
    *buf += len;

    // check if it is supposed to be a label
    ho_match_whitespace(buf);
    res = ho_match_string(":", buf);
    if (res != NO_ERR)
    {
    ho_parse_label_not_label:
        *buf = line_start;
        return ERR_NO_MATCH;
    }

    // check if appropriate
    if (ho_symbol_exists(*program, ident))
        return ERR_REDEFINED_IDENT;

    ho_add_symbol(program, ident, program->data_offset + program->len_data + program->len_code_lines + program->len_extra_macro_code, HO_SYM_LABEL);

    return NO_ERR;
}

// Match "reg reg" or "reg reg reg"
int ho_parse_format_2(horizon_program_t *program, char **buf)
{
    char *start = *buf;
    uint32_t regnum;
    int res = ho_match_register(&regnum, buf);
    if (res != NO_ERR)
        return res;

    program->args[0] = regnum;

    res = ho_match_register(&regnum, buf);
    if (res != NO_ERR)
        return res;

    program->args[1] = regnum;

    // If this gives no match and instead whitespace matches, the previous registers
    // were instead 0/1 and 2 instead of 0 and 1
    res = ho_match_register(&regnum, buf);
    if (res != NO_ERR)
    {
        // After this there should be no more arguments
        ho_match_whitespace(buf);
        ho_match_comment(NULL, buf);
        if (**buf != '\n' && **buf != '\0')
        {
            *buf = start;
            return ERR_NO_MATCH;
        }

        program->args[2] = program->args[1];
        program->args[1] = program->args[0];
        return NO_ERR;
    }

    program->args[2] = regnum;

    return NO_ERR;
}

// Match "reg imm8" or "reg reg imm8"
int ho_parse_format_3(horizon_program_t *program, char **buf)
{
    char *start = *buf;
    uint32_t regnum;
    int res = ho_match_register(&regnum, buf);
    if (res != NO_ERR)
        return res;

    program->args[0] = regnum;

    // If this does not match, the first argument is the same as the destination
    res = ho_match_register(&regnum, buf);
    if (res != NO_ERR)
    {
        program->args[1] = program->args[0];
    }
    else
    {
        program->args[1] = regnum;
    }

    uint32_t imm8;
    res = ho_match_imm8(&imm8, buf);
    if (res != NO_ERR)
        return res;

    // After this there should be no more arguments
    ho_match_whitespace(buf);
    ho_match_comment(NULL, buf);
    if (**buf != '\n' && **buf != '\0')
    {
        *buf = start;
        return ERR_NO_MATCH;
    }
    program->args[2] = imm8;

    return NO_ERR;
}

// Match "reg"
int ho_parse_format_4(horizon_program_t *program, char **buf)
{
    char *start = *buf;
    uint32_t regnum;
    int res = ho_match_register(&regnum, buf);
    if (res != NO_ERR)
        return res;

    // After this there should be no more arguments
    ho_match_whitespace(buf);
    ho_match_comment(NULL, buf);
    if (**buf != '\n' && **buf != '\0')
    {
        *buf = start;
        return ERR_NO_MATCH;
    }

    program->args[0] = regnum;
    program->args[1] = 0;
    program->args[2] = 0;

    return NO_ERR;
}

// Match "imm16"
int ho_parse_format_5(horizon_program_t *program, char **buf)
{
    char *start = *buf;
    uint32_t imm16;
    int res = ho_match_imm16(&imm16, buf);

    if (res != NO_ERR)
        return res;

    // After this there should be no more arguments
    ho_match_whitespace(buf);
    ho_match_comment(NULL, buf);
    if (**buf != '\n' && **buf != '\0')
    {
        *buf = start;
        return ERR_NO_MATCH;
    }

    program->args[0] = imm16;
    program->args[1] = 0;
    program->args[2] = 0;

    return NO_ERR;
}

// Match "reg" or "reg reg"
int ho_parse_format_6(horizon_program_t *program, char **buf)
{
    char *start = *buf;
    uint32_t regnum;
    int res = ho_match_register(&regnum, buf);
    if (res != NO_ERR)
        return res;

    program->args[0] = regnum;

    // If this gives no match and instead whitespace matches, the previous register
    // was instead 0/1 instead of 0
    res = ho_match_register(&regnum, buf);
    if (res != NO_ERR)
    {
        // After this there should be no more arguments
        ho_match_whitespace(buf);
        ho_match_comment(NULL, buf);
        if (**buf != '\n' && **buf != '\0')
        {
            *buf = start;
            return ERR_NO_MATCH;
        }

        program->args[1] = program->args[0];
        return NO_ERR;
    }

    program->args[1] = regnum;

    return NO_ERR;
}

int ho_parse_alu(horizon_program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int ho_parse_ram(horizon_program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int ho_parse_cond(horizon_program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

int ho_parse_push(horizon_program_t *program, char **buf)
{
    return ERR_NOT_IMPLEMENTED;
}

// Increments program->len_code if the next line starts with an instruction
// For the first parsing pass
int ho_count_instruction(horizon_program_t *program, char **buf)
{
    int res = 0;

    // If any of the instructions are matched, count up and match_error to consume the line
    char *bufpos = *buf;
    uint32_t opcode;
    res = ho_match_noop(&opcode, buf);
    if (res == NO_ERR)
    {
        ho_add_code_line(program, bufpos);
        return NO_ERR;
    }

    res = ho_match_alu(&opcode, buf);
    if (res == NO_ERR)
    {
        ho_add_code_line(program, bufpos);
        return NO_ERR;
    }

    res = ho_match_pop(&opcode, buf);
    if (res == NO_ERR)
    {
        ho_add_code_line(program, bufpos);
        return NO_ERR;
    }

    res = ho_match_not(&opcode, buf);
    if (res == NO_ERR)
    {
        ho_add_code_line(program, bufpos);
        return NO_ERR;
    }

    res = ho_match_mem(&opcode, buf);
    if (res == NO_ERR)
    {
        ho_add_code_line(program, bufpos);
        return NO_ERR;
    }

    res = ho_match_cond(&opcode, buf);
    if (res == NO_ERR)
    {
        ho_add_code_line(program, bufpos);
        return NO_ERR;
    }

    res = ho_match_push(&opcode, buf);
    if (res == NO_ERR)
    {
        ho_add_code_line(program, bufpos);
        return NO_ERR;
    }

    for (int i = 0; i < program->len_macros; i++)
    {
        res = ho_match_string(program->macros[i].name, buf);
        if (res == NO_ERR)
        {
            ho_add_code_line(program, bufpos);
            program->len_extra_macro_code += program->macros[i].len - 1;
            return NO_ERR;
        }
    }

    return res;
}

// Parse instructions into machine code
// This function does not use the program buffer anymore, it should be given
// a single line with a single instruction or macro
int ho_parse_instruction(horizon_program_t *program, char *buf)
{
    char *start = buf;
    int res;
    uint32_t opcode;

    // For the simple instructions no complex parsing is necessary
    res = ho_match_noop(&opcode, &buf);
    if (res == NO_ERR)
    {
        int64_t instr = opcode << 24;
        ho_add_code(program, instr);
    }

    // ALU instructions
    res = ho_match_alu(&opcode, &buf);
    if (res == NO_ERR)
    {
        res = ho_parse_format_2(program, &buf);
        if (res != NO_ERR)
        {
            res = ho_parse_format_3(program, &buf);
            if (res != NO_ERR)
            {
                return ERR_EXPECTED_FORMAT_2_3;
            }
        }

        int64_t instr = opcode << 24 | program->args[2] << 16 | program->args[1] << 8 | program->args[0];
        ho_add_code(program, instr);
    }

    return ERR_NOT_IMPLEMENTED;
}

// Parses a non-empty line, which can be an instruction, a directive or a label.
// If it is the .desc directive, multiple lines could be consumed, so lines_consumed will
// be set to the appropriate count, or 0 otherwise.
int ho_parse_statement(horizon_program_t *program, int *lines_consumed, char **buf)
{
    int retval = NO_ERR;

    *lines_consumed = 0;
    retval = ho_parse_directive(program, lines_consumed, buf);

    if (retval == ERR_EXPECTED_CLOSE_CB)
        return retval;
    if (retval == NO_ERR || retval != ERR_NO_MATCH)
        return retval;

    retval = ho_parse_label(program, buf);

    if (retval == NO_ERR || retval != ERR_NO_MATCH)
        return retval;

    retval = ho_count_instruction(program, buf);
    if (retval == ERR_NO_MATCH)
        return ERR_UNKNOWN_INSTRUCTION;
    // consume the line, this instruction will be parsed in the second pass
    if (retval == NO_ERR)
        ho_match_error_no_nl(buf);

    return retval;
}


// Print an error message for a parser error
void ho_parser_perror(char *msg, int error, int line)
{
    if (error == NO_ERR)
        return;

    printf("Error on line %d: ", line);

    switch (error)
    {
        case ERR_NOT_IMPLEMENTED:
            printf("not implemented yet");
            break;
        case ERR_NO_MATCH:
            printf("no match found");
            break;
        case ERR_OUT_OF_RANGE_8:
            printf("number out of range (less than %d or greater than %u)", INT8_MIN, UINT8_MAX);
            break;
        case ERR_OUT_OF_RANGE_16:
            printf("number out of range (less than %d or greater than %u)", INT16_MIN, UINT16_MAX);
            break;
        case ERR_OUT_OF_RANGE_32:
            printf("number out of range (less than %d or greater than %u)", INT32_MIN, UINT32_MAX);
            break;
        case ERR_UNEXPECTED_NL:
            printf("unexpected newline");
            break;
        case ERR_EOF:
            printf("found EOF");
            break;
        case ERR_IDENT_TOO_LONG:
            printf("identifier too long (max 255 characters)");
            break;
        case ERR_RESERVED_WORD:
            printf("cannot use reserved word as identifier");
            break;
        case ERR_REDEFINED_IDENT:
            printf("identifier was already defined");
            break;
        case ERR_EXPECTED_IDENT:
            printf("expected an identifier");
            break;
        case ERR_EXPECTED_LITERAL:
            printf("expected a literal value");
            break;
        case ERR_EXPECTED_CONST_OR_LITERAL:
            printf("expected a literal or constant value");
            break;
        case ERR_ILLEGAL_DATA_DIRECTIVE:
            printf("var and array directives must be defined before any instructions");
            break;
        case ERR_EXPECTED_NON_ZERO:
            printf("expected a non-zero value");
            break;
        case ERR_TOO_MANY_VALUES:
            printf("got more values than expected");
            break;
        case ERR_EXPECTED_OPEN_P:
            printf("expected (");
            break;
        case ERR_EXPECTED_CLOSE_P:
            printf("expected )");
            break;
        case ERR_EXPECTED_OPEN_B:
            printf("expected [");
            break;
        case ERR_EXPECTED_CLOSE_B:
            printf("expected ]");
            break;
        case ERR_EXPECTED_OPEN_CB:
            printf("expected {");
            break;
        case ERR_EXPECTED_CLOSE_CB:
            printf("expected }");
            break;
        case ERR_EXPECTED_COLON:
            printf("expected :");
            break;
        case ERR_UNKNOWN_INSTRUCTION:
            printf("unknown instruction");
            break;
        case ERR_UNKNOWN_DIRECTIVE:
            printf("unknown directive");
            break;
        case ERR_TOO_MANY_ARGUMENTS:
            printf("got more arguments than expected");
            break;
        case ERR_EXPECTED_COMMENT:
            printf("expected a comment");
            break;
        case ERR_EXPECTED_MACRO:
            printf("macros need at least 1 instruction");
            break;
        case ERR_EXPECTED_FORMAT_1:
            printf("instruction takes no arguments");
            break;
        case ERR_EXPECTED_FORMAT_2_3:
            printf("instruction takes Rd [Rn] Rm/imm8 arguments");
            break;
        case ERR_EXPECTED_FORMAT_4:
            printf("instruction takes one register argument");
            break;
        case ERR_EXPECTED_FORMAT_4_5:
            printf("instruction takes one imm16 argument");
            break;
        default:
            printf("%d", error);
    }

    printf("\n%s%s\n", (msg) ? msg : "", (msg && strlen(msg) != 0) ? "\n" : "");
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

// // Returns 1 if the word string is a reserved identifier, and 0 if not
// int ho_is_reserved_ident(const char *ident)
// {
//     for (int i = 0; i < sizeof(horizon_reserved_ident)/sizeof(char*); i++)
//     {
//         if (strcmp(ident, horizon_reserved_ident[i]) == 0)
//             return 1;
//     }
//     return 0;
// }
