#ifndef HORIZON_PARSER_H
#define HORIZON_PARSER_H

#include <stdint.h>
#include "../program.h"

// Grammar rules

// Match means advance the buffer and insert the matched token's value into dest
// Return values are errors

int match_literal(uint32_t *dest, char **buf);
int match_imm8(uint32_t *dest, char **buf);
int match_imm16(uint32_t *dest, char **buf);
int match_register(uint32_t *dest, char **buf);
int match_identifier(uint32_t *dest, char **buf);
int match_new_identifier(uint32_t *dest, char **buf);
int match_whitespace(char **buf);
int match_comment(char **buf);
int match_error(char **buf);

int match_noop(uint32_t *dest, char **buf);
int match_not_pop(uint32_t *dest, char **buf);
int match_alu(uint32_t *dest, char **buf);
int match_push(uint32_t *dest, char **buf);
int match_cond(uint32_t *dest, char **buf);
int match_mem(uint32_t *dest, char **buf);

// Parse means this is a rule in the grammar
// Return values are errors

int parse_value(program_t *program, char **buf);
int parse_value_list(program_t *program, char **buf);
int parse_directive(program_t *program, char **buf);
int parse_label(program_t *program, char **buf);
int parse_format_2(program_t *program, char **buf);
int parse_format_3(program_t *program, char **buf);
int parse_format_4(program_t *program, char **buf);
int parse_format_5(program_t *program, char **buf);
int parse_alu(program_t *program, char **buf);
int parse_ram(program_t *program, char **buf);
int parse_cond(program_t *program, char **buf);
int parse_push(program_t *program, char **buf);
int parse_instruction(program_t *program, char **buf);
int parse_statement(program_t *program, char **buf);


#endif // HORIZON_PARSER_H
