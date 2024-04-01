#ifndef HORIZON_PARSER_H
#define HORIZON_PARSER_H

#include <stdint.h>
#include "../program.h"

#define ERR_NO_MATCH        100
#define ERR_OUT_OF_RANGE_8  101
#define ERR_OUT_OF_RANGE_16 102
#define ERR_OUT_OF_RANGE_32 103
#define ERR_UNEXPECTED_NL   104
#define ERR_EOF             105
#define ERR_IDENT_TOO_LONG  106
#define ERR_RESERVED_WORD   107

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

int ho_parse_value(program_t *program, char **buf);
int ho_parse_value_list(program_t *program, char **buf);
int ho_parse_directive(program_t *program, char **buf);
int ho_parse_label(program_t *program, char **buf);
int ho_parse_format_2(program_t *program, char **buf);
int ho_parse_format_3(program_t *program, char **buf);
int ho_parse_format_4(program_t *program, char **buf);
int ho_parse_format_5(program_t *program, char **buf);
int ho_parse_alu(program_t *program, char **buf);
int ho_parse_ram(program_t *program, char **buf);
int ho_parse_cond(program_t *program, char **buf);
int ho_parse_push(program_t *program, char **buf);
int ho_parse_instruction(program_t *program, char **buf);
int ho_parse_statement(program_t *program, char **buf);

// Helper functions

void ho_parser_perror(char *msg, int error);
int ho_is_reserved(const char *word);
int ho_is_reserved_ident(const char *ident);

#endif // HORIZON_PARSER_H
