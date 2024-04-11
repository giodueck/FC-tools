#ifndef HORIZON_PARSER_H
#define HORIZON_PARSER_H

#include <regex.h>
#include <stdint.h>
#include "../program.h"

#define ERR_NO_MATCH                    100
#define ERR_OUT_OF_RANGE_8              101
#define ERR_OUT_OF_RANGE_16             102
#define ERR_OUT_OF_RANGE_32             103
#define ERR_UNEXPECTED_NL               104
#define ERR_EOF                         105
#define ERR_IDENT_TOO_LONG              106
#define ERR_RESERVED_WORD               107
#define ERR_REDEFINED_IDENT             108
#define ERR_EXPECTED_IDENT              109
#define ERR_EXPECTED_LITERAL            110
#define ERR_EXPECTED_CONST_OR_LITERAL   111
#define ERR_ILLEGAL_DATA_DIRECTIVE      112

#define HORIZON_IDENT_MAX_LEN 255

typedef struct {
    int arch;

    int len_symbols;
    int len_symbols_space;
    symbol_t *symbols;      // malloced

    // Variables
    int data_offset;        // for var and array directives
    int len_data;
    int len_data_space;
    uint32_t *data;         // malloced

    char *lines_buf;        // malloced
    int len_lines;

    int code_offset;        // for labels
    int code_start;         // for the initial jmp start instruction
    int len_code;
    int64_t *code;          // malloced

    int error_count;
} horizon_program_t;

struct horizon_regex_t {
    regex_t literal_re;
    regex_t register_re;
    regex_t identifier_re;
    regex_t instruction_re;
    regex_t directive_re;
};

static struct horizon_regex_t horizon_regex = { 0 };

extern const char *horizon_reserved[];
extern const char *horizon_reserved_ident[];

enum horizon_opcode {
    HO_ADD =       0,
    HO_SUB =       1,
    HO_MUL =       2,
    HO_DIV =       3,
    HO_MOD =       4,
    HO_EXP =       5,
    HO_LSH =       6,
    HO_RSH =       7,
    HO_AND =       8,
    HO_OR =        9,
    HO_NOT =       10,
    HO_XOR =       11,
    HO_BCAT =      12,
    HO_HCAT =      13,
    HO_ADDS =      16,
    HO_SUBS =      17,
    HO_MULS =      18,
    HO_DIVS =      19,
    HO_MODS =      20,
    HO_EXPS =      21,
    HO_LSHS =      22,
    HO_RSHS =      23,
    HO_ANDS =      24,
    HO_ORS =       25,
    HO_NOTS =      26,
    HO_XORS =      27,
    HO_BCATS =     28,
    HO_HCATS =     29,
    HO_JEQ =       32,
    HO_JNE =       33,
    HO_JLT =       34,
    HO_JGT =       35,
    HO_JLE =       36,
    HO_JGE =       37,
    HO_JNG =       38,
    HO_JPZ =       39,
    HO_JVS =       40,
    HO_JVC =       41,
    HO_JMP =       42,
    HO_NOOP =      43,
    HO_STORE =     48,
    HO_LOAD =      49,
    HO_STOREI =    50,
    HO_LOADI =     51,
    HO_STORED =    52,
    HO_LOADD =     53,
    HO_PUSH =      56,
    HO_POP =       57
};

enum horizon_directive {
    HO_CONST,
    HO_VAR,
    HO_ARRAY,
    HO_START,
    HO_NAME,
    HO_DESC,
    HO_DIR_NONE = -1,
};

enum horizon_symbol_type {
    HO_SYM_CONST,
    HO_SYM_VAR,
    HO_SYM_LABEL,
};

// Grammar rules

// Match means advance the buffer and insert the matched token's value into dest
// Return values are errors

int ho_match_literal(uint32_t *dest, char **buf);
int ho_match_imm8(uint32_t *dest, char **buf);
int ho_match_imm16(uint32_t *dest, char **buf);
int ho_match_register(uint32_t *dest, char **buf);
int ho_match_identifier(uint32_t *dest, char **buf);
int ho_match_directive(uint32_t *dest, char **buf);
int ho_match_whitespace(char **buf);
int ho_match_newline(char **buf);
int ho_match_comment(char **buf);
int ho_match_error(char **buf);

int ho_match_noop(uint32_t *dest, char **buf);
int ho_match_not_pop(uint32_t *dest, char **buf);
int ho_match_alu(uint32_t *dest, char **buf);
int ho_match_push(uint32_t *dest, char **buf);
int ho_match_cond(uint32_t *dest, char **buf);
int ho_match_mem(uint32_t *dest, char **buf);

// Parse means this is a rule in the grammar
// Return values are errors

int ho_parse_value(horizon_program_t *program, uint32_t *dest, char **buf);
int ho_parse_value_list(horizon_program_t *program, uint32_t **dest_list, int len, char **buf);
int ho_parse_directive(horizon_program_t *program, int *lines_consumed, char **buf);
int ho_parse_label(horizon_program_t *program, char **buf);
int ho_parse_format_2(horizon_program_t *program, char **buf);
int ho_parse_format_3(horizon_program_t *program, char **buf);
int ho_parse_format_4(horizon_program_t *program, char **buf);
int ho_parse_format_5(horizon_program_t *program, char **buf);
int ho_parse_alu(horizon_program_t *program, char **buf);
int ho_parse_ram(horizon_program_t *program, char **buf);
int ho_parse_cond(horizon_program_t *program, char **buf);
int ho_parse_push(horizon_program_t *program, char **buf);
int ho_parse_instruction(horizon_program_t *program, char **buf);
int ho_parse_statement(horizon_program_t *program, int *lines_consumed, char **buf);

// Helper functions

int ho_symbol_exists(horizon_program_t program, const char *token);
int ho_add_symbol(horizon_program_t *program, const char *ident, uint32_t value, int type);
void ho_parser_perror(char *msg, int error, int line);
int ho_is_reserved(const char *word);
int ho_is_reserved_ident(const char *ident);

#endif // HORIZON_PARSER_H
