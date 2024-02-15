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
} program_t;

#endif // FC_PROGRAM_H
