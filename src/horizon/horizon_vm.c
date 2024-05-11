#include "horizon_vm.h"
#include "horizon_parser.h"
#include <math.h>

void hovm_write_reg(horizon_vm_t *vm, uint8_t reg, uint32_t value)
{
    switch (reg)
    {
        case HO_NIL:
            return;

        default:
            vm->registers[reg] = value;
    }
}

int32_t hovm_read_reg(horizon_vm_t *vm, uint8_t reg)
{
    switch (reg)
    {
        case HO_NIL:
            return 0;

        default:
            return vm->registers[reg];
    }
}

// Execute any of the ALU operations, with or without flags, with or without
// immediate arguments
void hovm_execute_alu(horizon_vm_t *vm, uint32_t ir)
{
    int set_flags = ir & 0x10000000;
    int imm_arg = ir & 0x80000000;
    int rd = (ir >> 16) & 0xFF;
    int rm = (ir >> 8) & 0xFF;
    int rn = ir & 0xFF;
    int8_t imm8 = ir & 0xFF;
    int res = 0;
    int A, B;

    switch ((ir >> 24) & 0x7F)
    {
        case HO_ADD: case HO_ADDS:
            A = hovm_read_reg(vm, rm);
            B = (imm_arg) ? imm8 : hovm_read_reg(vm, rn);
            res = A + B;

            if (set_flags)
                vm->v = ((1 - ((A < 0) ^ (B < 0))) & ((A < 0) ^ (res < 0)));

            hovm_write_reg(vm, rd, res);
            break;

        case HO_SUB: case HO_SUBS:
            A = hovm_read_reg(vm, rm);
            B = (imm_arg) ? imm8 : hovm_read_reg(vm, rn);
            res = A - B;

            if (set_flags)
                vm->v = (((A < 0) ^ (B < 0)) & ((A < 0) ^ (res < 0)));

            hovm_write_reg(vm, rd, res);
            break;

        case HO_MUL: case HO_MULS:
            A = hovm_read_reg(vm, rm);
            B = (imm_arg) ? imm8 : hovm_read_reg(vm, rn);
            res = A * B;

            hovm_write_reg(vm, rd, res);
            break;

        case HO_DIV: case HO_DIVS:
            A = hovm_read_reg(vm, rm);
            B = (imm_arg) ? imm8 : hovm_read_reg(vm, rn);
            res = A / B;

            hovm_write_reg(vm, rd, res);
            break;

        case HO_MOD: case HO_MODS:
            A = hovm_read_reg(vm, rm);
            B = (imm_arg) ? imm8 : hovm_read_reg(vm, rn);
            res = A % B;

            hovm_write_reg(vm, rd, res);
            break;

        case HO_EXP: case HO_EXPS:
            A = hovm_read_reg(vm, rm);
            B = (imm_arg) ? imm8 : hovm_read_reg(vm, rn);
            res = pow(A, B);

            hovm_write_reg(vm, rd, res);
            break;

        case HO_LSH: case HO_LSHS:
            A = hovm_read_reg(vm, rm);
            B = (imm_arg) ? imm8 : hovm_read_reg(vm, rn);
            res = A << B;

            hovm_write_reg(vm, rd, res);
            break;

        case HO_RSH: case HO_RSHS:
            A = hovm_read_reg(vm, rm);
            B = (imm_arg) ? imm8 : hovm_read_reg(vm, rn);
            res = A >> B;

            hovm_write_reg(vm, rd, res);
            break;

        case HO_AND: case HO_ANDS:
            A = hovm_read_reg(vm, rm);
            B = (imm_arg) ? imm8 : hovm_read_reg(vm, rn);
            res = A & B;

            hovm_write_reg(vm, rd, res);
            break;

        case HO_OR: case HO_ORS:
            A = hovm_read_reg(vm, rm);
            B = (imm_arg) ? imm8 : hovm_read_reg(vm, rn);
            res = A | B;

            hovm_write_reg(vm, rd, res);
            break;

        case HO_NOT: case HO_NOTS:
            A = hovm_read_reg(vm, rm);
            res = ~A;

            hovm_write_reg(vm, rd, res);
            break;

        case HO_XOR: case HO_XORS:
            A = hovm_read_reg(vm, rm);
            B = (imm_arg) ? imm8 : hovm_read_reg(vm, rn);
            res = A ^ B;

            hovm_write_reg(vm, rd, res);
            break;

        case HO_BCAT: case HO_BCATS:
            A = hovm_read_reg(vm, rm);
            B = (imm_arg) ? imm8 : hovm_read_reg(vm, rn);
            res = (A << 8) | B;

            hovm_write_reg(vm, rd, res);
            break;

        case HO_HCAT: case HO_HCATS:
            A = hovm_read_reg(vm, rm);
            B = (imm_arg) ? imm8 : hovm_read_reg(vm, rn);
            res = (A << 16) | B;

            hovm_write_reg(vm, rd, res);
            break;

        default:
            break;
    }

    if (set_flags)
    {
        vm->z = (res == 0);
        vm->n = (res < 0);
    }
}

// Execute any jump instruction, with or without immediate address
// Set PC to the jump argument if the condition is true, or increment if false
void hovm_execute_cond(horizon_vm_t *vm, uint32_t ir)
{
    int imm_arg = ir & 0x80000000;
    int rn = (ir >> 8) & 0xFF;
    uint16_t imm16 = ir & 0xFFFF;

    uint16_t A = (imm_arg) ? imm16 : hovm_read_reg(vm, rn);

    vm->registers[HO_PC]++;

    switch ((ir >> 24) & 0x7F)
    {
        case HO_JEQ:
            if (vm->z) vm->registers[HO_PC] = A;
            break;
        case HO_JNE:
            if (!vm->z) vm->registers[HO_PC] = A;
            break;
        case HO_JLT:
            if (vm->n != vm->v) vm->registers[HO_PC] = A;
            break;
        case HO_JGT:
            if (!vm->z && vm->n == vm->v) vm->registers[HO_PC] = A;
            break;
        case HO_JLE:
            if (vm->z && vm->n != vm->v) vm->registers[HO_PC] = A;
            break;
        case HO_JGE:
            if (vm->n == vm->v) vm->registers[HO_PC] = A;
            break;
        case HO_JNG:
            if (vm->n) vm->registers[HO_PC] = A;
            break;
        case HO_JPZ:
            if (!vm->n) vm->registers[HO_PC] = A;
            break;
        case HO_JVS:
            if (vm->v) vm->registers[HO_PC] = A;
            break;
        case HO_JVC:
            if (!vm->v) vm->registers[HO_PC] = A;
            break;
        case HO_JMP:
            vm->registers[HO_PC] = A;
            break;
    }
}

void hovm_execute_mem(horizon_vm_t *vm, uint32_t ir)
{
    int imm_arg = ir & 0x80000000;
    int rn = (ir >> 8) & 0xFF;
    int rd = (ir >> 16) & 0xFF;
    uint16_t imm16 = ir & 0xFFFF;

    uint16_t A = (imm_arg) ? imm16 : hovm_read_reg(vm, rn);

    uint32_t op = (ir >> 24);
    uint32_t ar = hovm_read_reg(vm, HO_AR);

    switch (op & 0x7F)
    {
        case HO_STORE:
        case HO_STOREI:
        case HO_STORED:
            if (ar >= 0 && ar < HOVM_RAM_SIZE)
                vm->ram[ar] = A;
            break;
        case HO_LOAD:
        case HO_LOADI:
        case HO_LOADD:
            if (ar >= 0 && ar < HOVM_RAM_SIZE)
                hovm_write_reg(vm, rd, vm->ram[ar]);
            break;
    }

    // STOREI and LOADI
    if (op & 2)
        vm->registers[HO_AR]++;
    // STORED and LOADD
    else if (op & 4)
        vm->registers[HO_AR]--;
}

// Load program into the first addresses in the VM's RAM
// Returns number of words written
int hovm_load_rom(horizon_vm_t *vm, uint32_t *program, size_t size)
{
    int i;
    for (i = 0; i < size && i < HOVM_ROM_SIZE; i++)
    {
        vm->ram[i] = program[i];
    }

    return i;
}

// Set PC to the first instruction, i.e. 0
int hovm_reset(horizon_vm_t *vm)
{
    vm->registers[HO_PC] = 0;
    return 0;
}

// Start execution from the start of the program
int hovm_run(horizon_vm_t *vm)
{
    // Instruction register
    uint32_t ir = 0;
    uint8_t op = 0;

    while (1)
    {
        // Get next instruction using PC
        ir = vm->ram[vm->registers[HO_PC]];

        // HALT = JMP PC
        if (ir == 0x2A000F00)
            return vm->registers[HO_R0];

        // Decode and execute
        switch ((ir >> 24) & 0x7F)  // ignore imm flag for this step of decoding
        {
            case HO_NOOP:
                vm->registers[HO_PC]++;
                break;
            case HO_ADD: case HO_ADDS:
            case HO_SUB: case HO_SUBS:
            case HO_MUL: case HO_MULS:
            case HO_DIV: case HO_DIVS:
            case HO_MOD: case HO_MODS:
            case HO_EXP: case HO_EXPS:
            case HO_LSH: case HO_LSHS:
            case HO_RSH: case HO_RSHS:
            case HO_AND: case HO_ANDS:
            case HO_OR: case HO_ORS:
            case HO_NOT: case HO_NOTS:
            case HO_XOR: case HO_XORS:
            case HO_BCAT: case HO_BCATS:
            case HO_HCAT: case HO_HCATS:
                hovm_execute_alu(vm, ir);
                vm->registers[HO_PC]++;
                break;
            case HO_JEQ:
            case HO_JNE:
            case HO_JLT:
            case HO_JGT:
            case HO_JLE:
            case HO_JGE:
            case HO_JNG:
            case HO_JPZ:
            case HO_JVS:
            case HO_JVC:
            case HO_JMP:
                hovm_execute_cond(vm, ir);
                break;
            case HO_STORE:
            case HO_LOAD:
            case HO_STOREI:
            case HO_LOADI:
            case HO_STORED:
            case HO_LOADD:
                hovm_execute_mem(vm, ir);
                vm->registers[HO_PC]++;
                break;
        }
    }
}
