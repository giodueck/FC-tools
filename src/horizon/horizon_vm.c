#include "horizon_vm.h"
#include "horizon_parser.h"
#include <math.h>

void hovm_write_reg(horizon_vm_t *vm, uint8_t reg, int32_t value)
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
void hovm_execute_alu(horizon_vm_t *vm, int32_t ir)
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

// Load program into the first addresses in the VM's RAM
// Returns number of words written
int hovm_load_rom(horizon_vm_t *vm, int32_t *program, size_t size)
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
    int32_t ir = 0;
    uint8_t op = 0;

    while (1)
    {
        // Get next instruction using PC
        ir = vm->ram[vm->registers[HO_PC]];

        // Decode and execute
        switch ((ir >> 24) & 0x7F)  // ignore imm flag for this step of decoding
        {
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
        }
    }
}
