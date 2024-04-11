#ifndef FC_PROGRAM_H
#define FC_PROGRAM_H

#include <stdint.h>

typedef struct {
    char *name;
    uint32_t value;

    int type;
    int length;
} symbol_t;

#define ARCH_OVERTURE   0
// Everest would be 1
#define ARCH_HORIZON    2

typedef struct {
    int line;
    int errno;
} error_t;

// Move into dedicated debugger header when that exists
enum db_command { RUN, STEP, STEP_INTO, CONTINUE, BREAKPOINT, DELETE_BREAKPOINT };

// Move into dedicated debugger header when that exists
enum db_break_type { BP_NORMAL, BP_END };

#endif // FC_PROGRAM_H
