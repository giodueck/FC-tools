#ifndef FC_OVERTURE_H
#define FC_OVERTURE_H

#include <stdint.h>
#include <stdio.h>

#include "overture_compiler.h"

typedef struct {
    uint32_t r0, r1, r2, r3, r4, r5;
} overture_register_map_t;

// Returns the current state of the registers
overture_register_map_t overture_get_registers();

// Sets the registers, useful for debugging
void overture_set_registers(overture_register_map_t reg_map);

// Set the program to parse and run
int overture_set_program(FILE *fd);

// Used to tell the program where to halt. If not used, the program will run forever
// If a negative value is given, the label "end" will be set as the end if it exists
// On success, returns the code word address for the given line, otherwise returns -1
int overture_set_end(int line);

// Runs the parsed program if it has no errors
// Command determines how far the program is allowed to run before it stops
int overture_run(int command);

#endif // FC_OVERTURE_H
