#ifndef HOPRIZON_VM_H
#define HOPRIZON_VM_H

#include <stdint.h>
#include <stdio.h>

#define HOVM_REGISTER_COUNT  255
#define HOVM_RAM_SIZE       4096
#define HOVM_RAM_CELL_SIZE    64
#define HOVM_ROM_SIZE       2048
#define HOVM_STACK_SIZE      512

typedef struct {
    uint32_t rev;
    uint32_t registers[HOVM_REGISTER_COUNT];
    uint8_t z, n, v;
    uint32_t ram[HOVM_RAM_SIZE];
    uint32_t stack[HOVM_STACK_SIZE];
    uint32_t cycles;

    // 1 if the corresponding code should break execution
    // 0 if not
    uint8_t breakpoint_map[HOVM_ROM_SIZE];
} horizon_vm_t;

enum horizon_vm_register {
    HO_R0,
    HO_R1,
    HO_R2,
    HO_R3,
    HO_R4,
    HO_R5,
    HO_R6,
    HO_R7,
    HO_R8,
    HO_R9,
    HO_R10,
    HO_R11,
    // Address Register
    HO_AR,
    // Stack Pointer
    HO_SP,
    // Link Register
    HO_LR,
    // Program Counter
    HO_PC,

    // Pseudo-register: voids writes and reads return 0
    HO_NIL = 255
};

// TODO: setup function for allocating vm memory depending on revision once SIMD,
//  RAM or GPIO expansions are implemented

// Load program into the first addresses in the VM's RAM
// Returns number of words written
int hovm_load_rom(horizon_vm_t *vm, uint32_t *program, size_t size);

// Set PC to the first instruction, i.e. 0
int hovm_reset(horizon_vm_t *vm);

// Start execution from the start of the program
// Stop only on HALT/JMP PC
void hovm_run(horizon_vm_t *vm);

// Start or resume execution of the program
// Stop on HALT/JMP PC or on a breakpoint
void hovm_continue(horizon_vm_t *vm);

// Execute one instruction
void hovm_step(horizon_vm_t *vm);

// Disassemble the word at the given address into its assembly equivalent
// If the address points to the data section between the first JMP and its
// destination, the resulting string is just the decimal representation of
// the value of the word.
// dest is assumed to be big enough
void hovm_disassemble(char *dest, horizon_vm_t *vm, uint32_t address);

#endif // HOPRIZON_VM_H
