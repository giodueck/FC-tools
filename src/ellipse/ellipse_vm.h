#ifndef ELLIPSE_VM_H
#define ELLIPSE_VM_H

#include <stdint.h>
#include <stdio.h>

#define ELVM_REGISTER_COUNT   32
#define ELVM_RAM_SIZE       8192
#define ELVM_RAM_CELL_SIZE    64
#define ELVM_ROM_SIZE       4096
#define ELVM_STACK_SIZE      512

#define ELVM_MODE_NORMAL 0
#define ELVM_MODE_RAMCP  1

typedef struct {
    uint32_t rev;
    uint32_t registers[ELVM_REGISTER_COUNT];
    uint8_t z, n, v;
    uint8_t mode;
    uint32_t ram[ELVM_RAM_SIZE];
    uint32_t stack[ELVM_STACK_SIZE];
    uint32_t cycles;

    uint8_t post_prng, post_mode_ramcp, post_mode_normal;

    // 1 if the corresponding code should break execution
    // 0 if not
    uint8_t breakpoint_map[ELVM_ROM_SIZE];
    // Set on ROM load, for dissassembly
    uint32_t program_size;
} ellipse_vm_t;

// Copy program into the first addresses in the VM's ROM
// Returns number of words written
int elvm_load_rom(ellipse_vm_t *vm, uint32_t *program, size_t size);

// Set PC to the first instruction, i.e. 0
int elvm_reset(ellipse_vm_t *vm);

// Start execution from the start of the program
// Stop only on HALT/JMP PC
void elvm_run(ellipse_vm_t *vm);

// Start or resume execution of the program
// Stop on HALT or on a breakpoint
void elvm_continue(ellipse_vm_t *vm);

// Execute one instruction
void elvm_step(ellipse_vm_t *vm);

// Disassemble the word at the given address into its assembly equivalent
// If the address points to the data section between the first JMP and its
// destination, the resulting string is just the decimal representation of
// the value of the word.
// dest is assumed to be big enough
void elvm_disassemble(char *dest, ellipse_vm_t *vm, int32_t address);

#endif // ELLIPSE_VM_H
