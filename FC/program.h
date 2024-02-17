#ifndef FC_PROGRAM_H
#define FC_PROGRAM_H

#include <stdint.h>

typedef struct {
    char *name;
    uint32_t value;
} symbol_t;

typedef struct {
    int len_symbols;
    symbol_t *symbols;
    int len_lines;
    char **lines;
    int *line_executable;
    int len_code;
    int64_t *code;
} program_t;

// Move into dedicated debugger header when that exists
enum db_command { RUN, STEP, STEP_INTO, CONTINUE, BREAKPOINT, DELETE_BREAKPOINT };

// Move into dedicated debugger header when that exists
enum db_break_type { BP_NORMAL, BP_END };

#endif // FC_PROGRAM_H
