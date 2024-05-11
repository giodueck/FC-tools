#include "horizon/horizon_parser.h"
#include "horizon/horizon_vm.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    uint32_t program[] = {
        0xAA000001,
        0x0B000000,
        0x8001FF03,
        0x800CFF40,
        0x32000100,
        0x91010101,
        0xA1000004,
        0xB0000000,
        0x0B0C0C0C,
        0x800CFF40,
        0x33010000,
        0x91FF0100,
        0xA000000F,
        0x00000001,
        0xAA00000A,
        0x2A000F00,
    };

    horizon_vm_t vm = { 0 };

    hovm_load_rom(&vm, program, sizeof(program) / sizeof(int32_t));
    hovm_run(&vm);

    return 0;
}
