#ifndef FC_OVERTURE_H
#define FC_OVERTURE_H

#define DEBUG

#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint32_t r0, r1, r2, r3, r4, r5;
} overture_register_map_t;

// Parse program to build the symbol table and check for any errors
// Returns 0 if no errors
// Call overture_init() before this function
// Allocates memory, free with overture_free()
int overture_parse(FILE *program);

// Frees up memory allocated during parsing
void overture_free();

// Interpret and execute an instruction
int overture_execute(const char *instr);

// Returns the current state of the registers
overture_register_map_t overture_get_registers();

// Sets the registers, useful for debugging
void overture_set_registers(overture_register_map_t reg_map);

// Prints all executable lines
void overture_print();

// Compiles the needed regular expressions
void overture_init();

// Frees the used regular expressions
void overture_quit();

// Runs the parsed program if it has no errors
int overture_run();

#endif // FC_OVERTURE_H
