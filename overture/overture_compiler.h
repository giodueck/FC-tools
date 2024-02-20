#ifndef OVERTURE_COMPILER_H
#define OVERTURE_COMPILER_H

#include <stdio.h>

#include "../program.h"

enum machine_code_def {
ADD = 64,
SUB = 65,
MUL = 66,
DIV = 67,
AND = 68,
ORR = 69,
NOT = 70,
XOR = 71,
NOP = 192,
JEQ = 193,
JLT = 194,
JLE = 195,
JMP = 196,
JNE = 197,
JGE = 198,
JGT = 199,
};

// Parse program to build the symbol table and check for any errors
// Returns 0 if no errors
// Call overture_init() before this function
// Allocates memory, free with overture_free()
int overture_parse(FILE *program);

// Frees up memory allocated during parsing
void overture_free();

// Prints all executable lines
void overture_print();

// Compiles the needed regular expressions
void overture_init();

// Frees the used regular expressions
void overture_quit();

// Returns the parsed program structure
// Note: this is still malloced memory, and calling overture_free will free it.
// Make sure not to use the program after it has been freed
program_t overture_get_program();


#endif // OVERTURE_COMPILER_H
