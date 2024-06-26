#ifndef HORIZON_COMPILER_H
#define HORIZON_COMPILER_H

#include <stdio.h>

#include "../program.h"
#include "horizon_parser.h"

// raw_output: outputs only binary (uint32 instructions) instead of BP strings
struct horizon_compiler_opt {
    uint8_t raw_output;
};

// Parse program to build the symbol table and check for errors
// Returns the pointer to a newly allocated horizon_program_t
// If there are any errors, they will be added to the err_array, as many as
// err_array_size allows. The horizon_program_t return will have the total error count.
// After running horizon_parse, run horizon_free on the pointer to free its
// allocated memory, regardless of if the program was error-free or not.
horizon_program_t *horizon_parse(FILE *program_fd, error_t *err_array, int err_array_size);

// Frees memory allocated by horizon_parse
void horizon_free(horizon_program_t *program);

// Parse the program at the given src_filepath and save the result in the given
// destination path dst_filename.
// The options struct changes the way the procedure operates
void horizon_compile(const char *dst_filename, const char *src_filename, struct horizon_compiler_opt *options);

#endif // HORIZON_COMPILER_H
