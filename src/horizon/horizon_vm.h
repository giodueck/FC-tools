#ifndef HOPRIZON_VM_H
#define HOPRIZON_VM_H

#include <stdint.h>
#include <stdio.h>

#define HOVM_REGISTER_COUNT  255
#define HOVM_RAM_SIZE       4096
#define HOVM_ROM_SIZE       2048

typedef struct {
    int rev;
    int32_t registers[256];
    int8_t z, n, v;
    int32_t ram[4096];
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
int hovm_load_rom(horizon_vm_t *vm, int32_t *program, size_t size);

// Set PC to the first instruction, i.e. 0
int hovm_reset(horizon_vm_t *vm);

// Start execution from the start of the program
int hovm_run(horizon_vm_t *vm);

#endif // HOPRIZON_VM_H
