#ifndef HORIZON_PARSER_H
#define HORIZON_PARSER_H

#include <regex.h>
#include <stdint.h>
#include "../program.h"

#ifndef DEBUG
#define DEBUG 0
#endif

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
#define ERR_EXPECTED_NON_ZERO           113
#define ERR_TOO_MANY_VALUES             114
#define ERR_EXPECTED_OPEN_P             115
#define ERR_EXPECTED_CLOSE_P            116
#define ERR_EXPECTED_OPEN_B             117
#define ERR_EXPECTED_CLOSE_B            118
#define ERR_EXPECTED_OPEN_CB            119
#define ERR_EXPECTED_CLOSE_CB           120
#define ERR_EXPECTED_COLON              121
#define ERR_UNKNOWN_INSTRUCTION         122
#define ERR_UNKNOWN_DIRECTIVE           123
#define ERR_TOO_MANY_ARGUMENTS          124
#define ERR_EXPECTED_COMMENT            125
#define ERR_EXPECTED_MACRO              126
#define ERR_EXPECTED_FORMAT_1           127
#define ERR_EXPECTED_FORMAT_2_3         128
#define ERR_EXPECTED_FORMAT_4           129
#define ERR_EXPECTED_FORMAT_4_5         130
#define ERR_EXPECTED_FORMAT_6           131
#define ERR_EXPECTED_IMM8               132
#define ERR_EXPECTED_IMM16              133
#define ERR_EXPECTED_INSTRUCTION        134
#define ERR_TOO_FEW_ARGUMENTS           135

#define HORIZON_IDENT_MAX_LEN 255

typedef struct {
    char name[HORIZON_IDENT_MAX_LEN + 1];
    int argc;
    int len;
    char **lines;
} horizon_macro_t;

typedef struct {
    int arch;

    // Symbols
    int len_symbols;
    int len_symbols_space;
    symbol_t *symbols;      // malloced

    // Variables in RAM
    int data_offset;        // for var and array directives
    int len_data;
    int len_data_space;
    uint32_t *data;         // malloced

    // Program text
    char *input_buf;        // malloced
    int len_input;

    // Instructions
    int curr_line;          // to store which line in the text corresponds to which 
                            // instruction
    int *code_line_indices; // malloced, same size as code_lines
    int code_offset;        // for labels
    int code_start;         // for the initial jmp start instruction
    int len_code_lines;
    int len_code_lines_space;
    char **code_lines;      // malloced, array of pointers to lines in the lines_buf, ending in '\n'
                            //  these lines must be parsed in the second pass to allow using labels defined
                            //  later

    // Machine code
    uint32_t args[3];       // arguments parsed are placed here temporarily
    int imm_arg;            // zero if no immediate arguments, 1 if there is
    int len_code;
    int len_code_space;
    int64_t *code;          // malloced

    int len_macros;
    int len_macros_space;
    horizon_macro_t *macros;    // malloced
    int len_extra_macro_code ;  // number of additional (over 1) code lines introduced by
                                // macros, for processing labels in first pass

    // Number of errors encountered
    int error_count;

    // Optional name and description
    char *name;             // points to somewhere in input_buf
    char *desc;             // malloced
} horizon_program_t;

struct horizon_regex_t {
    regex_t literal_re;
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
    HO_MACRO,
    HO_DIR_NONE = -1,
};

enum horizon_symbol_type {
    HO_SYM_CONST,
    HO_SYM_VAR,
    HO_SYM_MACRO,
    HO_SYM_LABEL,
};

// Grammar rules

// Initilizes all regex used by the parser
// If free_instead is not 0, frees the regex variables instead
int ho_init_regex(int free_instead);

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
int ho_match_comment(char **dest, char **buf);
int ho_match_string(const char *str, char **buf);
int ho_match_error(char **buf);

int ho_match_noop(uint32_t *dest, char **buf);
int ho_match_pop(uint32_t *dest, char **buf);
int ho_match_not(uint32_t *dest, char **buf);
int ho_match_alu(uint32_t *dest, char **buf);
int ho_match_push(uint32_t *dest, char **buf);
int ho_match_cond(uint32_t *dest, char **buf);
int ho_match_store(uint32_t *dest, char **buf);
int ho_match_load(uint32_t *dest, char **buf);

// Parse means this is a rule in the grammar
// Return values are errors

int ho_parse_value(horizon_program_t *program, uint32_t *dest, char **buf);
int ho_parse_value_list(horizon_program_t *program, uint32_t **dest_list, int len, char **buf);
int ho_parse_directive(horizon_program_t *program, int *lines_consumed, char **buf);
int ho_parse_macro(horizon_program_t *program, char *name, int argc, char **buf);
int ho_parse_label(horizon_program_t *program, char **buf);
int ho_parse_format_2(horizon_program_t *program, char **buf);
int ho_parse_format_3(horizon_program_t *program, char **buf);
int ho_parse_format_4(horizon_program_t *program, char **buf);
int ho_parse_format_5(horizon_program_t *program, char **buf);
int ho_parse_format_6(horizon_program_t *program, char **buf);
int ho_parse_alu(horizon_program_t *program, char **buf);
int ho_parse_ram(horizon_program_t *program, char **buf);
int ho_parse_cond(horizon_program_t *program, char **buf);
int ho_parse_push(horizon_program_t *program, char **buf);
int ho_valid_instruction(horizon_program_t *program, char **buf);
int ho_count_instruction(horizon_program_t *program, char **buf);
int ho_parse_instruction(horizon_program_t *program, char *buf);
int ho_parse_statement(horizon_program_t *program, int *lines_consumed, char **buf);

// Helper functions

int ho_add_builtin_macros(horizon_program_t *program);
int ho_symbol_exists(horizon_program_t program, const char *token);
int ho_add_symbol(horizon_program_t *program, const char *ident, uint32_t value, int type);
void ho_parser_perror(char *msg, int error, int line);
int ho_is_reserved(const char *word);
int ho_is_reserved_ident(const char *ident);

#endif // HORIZON_PARSER_H
