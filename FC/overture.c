#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include "overture.h"
#include "overture_compiler.h"
#include "program.h"

static program_t program = { 0 };
static int error_count = 0;

static overture_register_map_t reg = { 0 };
static int control_at = -1;
static int cycle = -1;
static int end_instruction = -1;
static int *breakpoints = NULL;


// Returns the current state of the registers
overture_register_map_t overture_get_registers()
{
    return reg;
}

// Sets the registers, useful for debugging
void overture_set_registers(overture_register_map_t reg_map)
{
    reg = reg_map;
}

// Used to tell the program where to halt. If not used, the program will run forever
// If a negative value is given, the label "end" will be set as the end if it exists
// On success, returns the code word address for the given line, otherwise returns -1
int overture_set_end(int line)
{
    end_instruction = -1;

    // Look for the label "end"
    if (line < 0)
    {
        for (int i = 0; i < program.len_symbols; i++)
        {
            if (!strcmp(program.symbols[i].name, "end"))
            {
                end_instruction = program.symbols[i].value;
                return end_instruction;
            }
        }
    }

    // If the line if after the last instruction, that's an error
    if (line > program.len_lines)
        return -1;

    // Look for the code word for the given line
    for (int i = 0; i < line; i++)
    {
        end_instruction += program.line_executable[i];
    }
    return end_instruction;
}

// Execute one instruction on the registers
static void overture_execute(int op, int *jump_addr)
{
    if (op < 0 || op > 255)
        return;
    if (op < 64)
    {
        reg.r0 = op;
        return;
    }

    switch (op)
    {
    case ADD:
        reg.r3 = reg.r1 + reg.r2;
        break;
    case SUB:
        reg.r3 = reg.r1 - reg.r2;
        break;
    case MUL:
        reg.r3 = reg.r1 * reg.r2;
        break;
    case DIV:
        if (reg.r2 == 0) reg.r3 = 0;
        else reg.r3 = reg.r1 / reg.r2;
        break;
    case AND:
        reg.r3 = reg.r1 & reg.r2;
        break;
    case ORR:
        reg.r3 = reg.r1 | reg.r2;
        break;
    case NOT:
        reg.r3 = ~reg.r1;
        break;
    case XOR:
        reg.r3 = reg.r1 ^ reg.r2;
        break;
    case NOP:
        break;
    case JEQ:
        if (reg.r3 == 0)
            *jump_addr = reg.r0;
        break;
    case JLT:
        if (reg.r3 < 0)
            *jump_addr = reg.r0;
        break;
    case JLE:
        if (reg.r3 <= 0)
            *jump_addr = reg.r0;
        break;
    case JMP:
        *jump_addr = reg.r0;
        break;
    case JNE:
        if (reg.r3 != 0)
            *jump_addr = reg.r0;
        break;
    case JGE:
        if (reg.r3 >= 0)
            *jump_addr = reg.r0;
        break;
    case JGT:
        if (reg.r3 > 0)
            *jump_addr = reg.r0;
        break;
    default:
        if ((op >> 6) == 2)
        {
            uint32_t rnull = 0;
            uint32_t *regarray[8] = { &reg.r0, &reg.r1, &reg.r2, &reg.r3, &reg.r4, &reg.r5, &rnull, &rnull };

            *regarray[op & 7] = *regarray[(op >> 3) & 7];
        }
        break;
    }
}

// Set the program to parse and run
int overture_set_program(FILE *fd)
{
    error_count = overture_parse(fd);
    program = overture_get_program();
    return error_count;
}

// If compiled with this program, the code will include '-1' where there was a
// compile-time error
int verify_binary()
{
    int errors = 0;
    for (int i = 0; i < program.len_code; i++)
    {
        if (program.code[i] < 0)
            errors++;
    }
    return errors;
}

// Runs the parsed program if it has no errors
// Command determines how far the program is allowed to run before it stops
int overture_run(int command)
{
    program = overture_get_program();
    if ((error_count = verify_binary()) != 0)
        return -1;

    switch (command)
    {
        case RUN:
            control_at = 0;
            while (control_at < program.len_code)
            {
                if (control_at == end_instruction)
                    break;
                int j = -1;
                overture_execute(program.code[control_at], &j);
                if (j >= 0)
                    control_at = j;
                else
                    control_at++;
                cycle++;
            }
            return 0;
        case STEP:
        case STEP_INTO:
        case CONTINUE:
        default:
            return -1;
    }
}
