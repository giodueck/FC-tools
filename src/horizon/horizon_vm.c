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
            if (B == 0)
                res = 0;
            else
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

void hovm_execute_stack(horizon_vm_t *vm, uint32_t ir)
{
    int imm_arg = ir & 0x80000000;
    int rn = (ir >> 8) & 0xFF;
    int rd = (ir >> 16) & 0xFF;
    uint16_t imm16 = ir & 0xFFFF;

    uint16_t A = (imm_arg) ? imm16 : hovm_read_reg(vm, rn);

    uint32_t op = (ir >> 24);
    uint32_t sp = hovm_read_reg(vm, HO_SP);

    switch (op & 0x7F)
    {
        case HO_PUSH:
            if (sp >= 0 && sp < HOVM_STACK_SIZE)
                vm->stack[sp] = A;
            sp++;
            break;
        case HO_POP:
            sp--;
            if (sp >= 0 && sp < HOVM_STACK_SIZE)
                hovm_write_reg(vm, rd, vm->stack[sp]);
            break;
    }
    hovm_write_reg(vm, HO_SP, sp);
}

// Load program into the first addresses in the VM's RAM
// Returns number of words written
int hovm_load_rom(horizon_vm_t *vm, uint32_t *program, size_t size)
{
    int i;
    vm->program_size = size;
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

// Execute one instruction
void hovm_step(horizon_vm_t *vm)
{
    // Instruction register
    uint32_t ir = 0;

    // Get next instruction using PC
    ir = vm->ram[vm->registers[HO_PC]];

    // HALT = JMP PC
    if (ir == HOVM_HALT)
        return;

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
        case HO_PUSH:
        case HO_POP:
            hovm_execute_stack(vm, ir);
            vm->registers[HO_PC]++;
            break;
    }
    vm->cycles++;
}

// Start execution from the start of the program
// Stop only on HALT/JMP PC
void hovm_run(horizon_vm_t *vm)
{
    // Instruction register
    uint32_t ir = 0;

    while (1)
    {
        // Get next instruction using PC
        ir = vm->ram[vm->registers[HO_PC]];

        // HALT = JMP PC
        if (ir == HOVM_HALT)
            return;

        // Execute instruction
        hovm_step(vm);
    }
}

// Start or resume execution of the program
// Stop on HALT/JMP PC or on a breakpoint
void hovm_continue(horizon_vm_t *vm)
{
    // Instruction register
    uint32_t ir = 0;

    while (1)
    {
        // Breakpoint
        if (vm->breakpoint_map[vm->registers[HO_PC]])
            return;

        // Get next instruction using PC
        ir = vm->ram[vm->registers[HO_PC]];

        // HALT = JMP PC
        if (ir == HOVM_HALT)
            return;

        // Execute instruction
        hovm_step(vm);
    }
}

const char *hovm_register_name(uint8_t reg)
{
    switch (reg)
    {
        case HO_R0:
            return "R0";
        case HO_R1:
            return "R1";
        case HO_R2:
            return "R2";
        case HO_R3:
            return "R3";
        case HO_R4:
            return "R4";
        case HO_R5:
            return "R5";
        case HO_R6:
            return "R6";
        case HO_R7:
            return "R7";
        case HO_R8:
            return "R8";
        case HO_R9:
            return "R9";
        case HO_R10:
            return "R10";
        case HO_R11:
            return "R11";
        case HO_AR:
            return "AR";
        case HO_SP:
            return "SP";
        case HO_LR:
            return "LR";
        case HO_PC:
            return "PC";
        case HO_NIL:
            return "NIL";
        default:
            return "";
    }
}

// Disassemble the word at the given address into its assembly equivalent
// If the address points to the data section between the first JMP and its
// destination, the resulting string is just the decimal representation of
// the value of the word.
// dest is assumed to be big enough
void hovm_disassemble(char *dest, horizon_vm_t *vm, int32_t address)
{
    uint32_t program_start = 0;

    // Get code section start
    if ((vm->ram[0] & 0xFF000000) == 0xAA000000)
        program_start = vm->ram[0] & 0xFFFF;

    if (address == 0)
    {
        sprintf(dest, "JMP #0x%X", program_start);
        return;
    }
    else if (address < 0 || address >= vm->program_size)
    {
        sprintf(dest, "-");
        return;
    }
    else if (address < program_start)
    {
        sprintf(dest, "%d", vm->ram[address]);
    }
    else if (address < HOVM_RAM_SIZE)
    {
        uint8_t op = (vm->ram[address] >> 24);
        uint8_t imm = (op & 0x80);
        uint8_t rd = (vm->ram[address] >> 16) & 0xFF;
        uint8_t rm = (vm->ram[address] >> 8) & 0xFF;
        uint8_t rn = vm->ram[address] & 0xFF;
        int8_t imm8 = rn;
        int16_t imm16 = vm->ram[address] & 0xFFFF;

        switch (op & 0x7F)
        {
            case HO_ADD:
                if (imm) sprintf(dest, "ADD %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "ADD %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_ADDS:
                if (imm) sprintf(dest, "ADDS %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "ADDS %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_SUB:
                if (imm) sprintf(dest, "SUB %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "SUB %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_SUBS:
                if (imm) sprintf(dest, "SUBS %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "SUBS %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_MUL:
                if (imm) sprintf(dest, "MUL %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "MUL %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_MULS:
                if (imm) sprintf(dest, "MULS %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "MULS %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_DIV:
                if (imm) sprintf(dest, "DIV %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "DIV %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_DIVS:
                if (imm) sprintf(dest, "DIVS %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "DIVS %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_MOD:
                if (imm) sprintf(dest, "MOD %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "MOD %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_MODS:
                if (imm) sprintf(dest, "MODS %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "MODS %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_EXP:
                if (imm) sprintf(dest, "EXP %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "EXP %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_EXPS:
                if (imm) sprintf(dest, "EXPS %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "EXPS %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_LSH:
                if (imm) sprintf(dest, "LSH %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "LSH %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_LSHS:
                if (imm) sprintf(dest, "LSHS %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "LSHS %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_RSH:
                if (imm) sprintf(dest, "RSH %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "RSH %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_RSHS:
                if (imm) sprintf(dest, "RSHS %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "RSHS %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_AND:
                if (imm) sprintf(dest, "AND %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "AND %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_ANDS:
                if (imm) sprintf(dest, "ANDS %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "ANDS %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_OR:
                if (imm) sprintf(dest, "OR %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "OR %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_ORS:
                if (imm) sprintf(dest, "ORS %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "ORS %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_NOT:
                if (imm) sprintf(dest, "NOT %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "NOT %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_NOTS:
                if (imm) sprintf(dest, "NOTS %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "NOTS %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_XOR:
                if (imm) sprintf(dest, "XOR %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "XOR %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_XORS:
                if (imm) sprintf(dest, "XORS %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "XORS %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_BCAT:
                if (imm) sprintf(dest, "BCAT %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "BCAT %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_BCATS:
                if (imm) sprintf(dest, "BCATS %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "BCATS %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_HCAT:
                if (imm) sprintf(dest, "HCAT %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "HCAT %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_HCATS:
                if (imm) sprintf(dest, "HCATS %s %s #%d", hovm_register_name(rd), hovm_register_name(rm), imm8);
                else sprintf(dest, "HCATS %s %s %s", hovm_register_name(rd), hovm_register_name(rm), hovm_register_name(rn));
                break;
            case HO_JEQ:
                if (imm) sprintf(dest, "JEQ #%x", imm16);
                else sprintf(dest, "JEQ %s", hovm_register_name(rm)); 
                break;
            case HO_JNE:
                if (imm) sprintf(dest, "JNE #%x", imm16);
                else sprintf(dest, "JNE %s", hovm_register_name(rm)); 
                break;
            case HO_JLT:
                if (imm) sprintf(dest, "JLT #%x", imm16);
                else sprintf(dest, "JLT %s", hovm_register_name(rm)); 
                break;
            case HO_JGT:
                if (imm) sprintf(dest, "JGT #%x", imm16);
                else sprintf(dest, "JGT %s", hovm_register_name(rm)); 
                break;
            case HO_JLE:
                if (imm) sprintf(dest, "JLE #%x", imm16);
                else sprintf(dest, "JLE %s", hovm_register_name(rm)); 
                break;
            case HO_JGE:
                if (imm) sprintf(dest, "JGE #%x", imm16);
                else sprintf(dest, "JGE %s", hovm_register_name(rm)); 
                break;
            case HO_JNG:
                if (imm) sprintf(dest, "JNG #%x", imm16);
                else sprintf(dest, "JNG %s", hovm_register_name(rm)); 
                break;
            case HO_JPZ:
                if (imm) sprintf(dest, "JPZ #%x", imm16);
                else sprintf(dest, "JPZ %s", hovm_register_name(rm)); 
                break;
            case HO_JVS:
                if (imm) sprintf(dest, "JVS #%x", imm16);
                else sprintf(dest, "JVS %s", hovm_register_name(rm)); 
                break;
            case HO_JVC:
                if (imm) sprintf(dest, "JVC #%x", imm16);
                else sprintf(dest, "JVC %s", hovm_register_name(rm)); 
                break;
            case HO_JMP:
                if (imm) sprintf(dest, "JMP #%x", imm16);
                else sprintf(dest, "JMP %s", hovm_register_name(rm)); 
                break;
            case HO_STORE:
                if (imm) sprintf(dest, "STORE #%d", imm16);
                else sprintf(dest, "STORE %s", hovm_register_name(rm));
                break;
            case HO_LOAD:
                if (imm) sprintf(dest, "ILLEGAL");
                else sprintf(dest, "LOAD %s", hovm_register_name(rd));
                break;
            case HO_STOREI:
                if (imm) sprintf(dest, "STOREI #%d", imm16);
                else sprintf(dest, "STOREI %s", hovm_register_name(rm));
                break;
            case HO_LOADI:
                if (imm) sprintf(dest, "ILLEGAL");
                else sprintf(dest, "LOADI %s", hovm_register_name(rd));
                break;
            case HO_STORED:
                if (imm) sprintf(dest, "STORED #%d", imm16);
                else sprintf(dest, "STORED %s", hovm_register_name(rm));
                break;
            case HO_LOADD:
                if (imm) sprintf(dest, "ILLEGAL");
                else sprintf(dest, "LOADD %s", hovm_register_name(rd));
                break;
            case HO_PUSH:
                if (imm) sprintf(dest, "PUSH #%d", imm16);
                else sprintf(dest, "PUSH %s", hovm_register_name(rm));
                break;
            case HO_POP:
                if (imm) sprintf(dest, "ILLEGAL");
                else sprintf(dest, "POP %s", hovm_register_name(rd));
                break;
            default:
                sprintf(dest, "ILLEGAL");
        }
    }
}

