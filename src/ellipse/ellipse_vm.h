#ifndef ELLIPSE_VM_H
#define ELLIPSE_VM_H

#include <stdint.h>
#include <stdio.h>

#define ELVM_REGISTER_COUNT   32
#define ELVM_RAM_SIZE       8192
#define ELVM_RAM_CELL_SIZE    64
#define ELVM_ROM_SIZE       4096
#define ELVM_STACK_SIZE     2048

#define ELVM_HALT 0x00000001

typedef struct {
    uint32_t rev;
    uint32_t registers[ELVM_REGISTER_COUNT];
    uint8_t z, n, v;
    uint32_t ram[ELVM_RAM_SIZE];
    uint32_t stack[ELVM_STACK_SIZE];
    uint32_t cycles;

    // 1 if the corresponding code should break execution
    // 0 if not
    uint8_t breakpoint_map[ELVM_ROM_SIZE];
    // Set on ROM load, for dissassembly
    uint32_t program_size;
} ellipse_vm_t;

enum ellipse_vm_register {
    // Hardwired zero
    EL_ZERO,
    // Stack Pointer
    EL_SP,
    // Return Address
    EL_RA,
    // Program Counter
    EL_PC,
    // IO Receive
    EL_RX,
    // IO Receive Size
    EL_RS,
    // IO Transmit
    EL_TX,
    // IO Chip Select
    EL_CS,
    // GP argument/return registers
    EL_A0,
    EL_A1,
    EL_A2,
    EL_A3,
    EL_A4,
    EL_A5,
    EL_A6,
    EL_A7,
    // GP saved registers
    EL_S0,
    EL_S1,
    EL_S2,
    EL_S3,
    EL_S4,
    EL_S5,
    EL_S6,
    EL_S7,
    // GP temporary registers
    EL_T0,
    EL_T1,
    EL_T2,
    EL_T3,
    EL_T4,
    EL_T5,
    // PRNG seed and next value
    EL_PRNG,
    // GP Ticker
    EL_TICK
};

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
