#include "horizon/horizon_parser.h"
#include "horizon/horizon_vm.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    uint32_t program[] = {
        0xaa000001,
        0x0b000000,
        0x8001ff04,
        0x38000100,
        0x91010101,
        0xa1000003,
        0x11ff0dff,
        0xa000000b,
        0x39010000,
        0x00000001,
        0xaa000006,
        0x2a000f00,
    };

    horizon_vm_t vm = { 0 };

    hovm_load_rom(&vm, program, sizeof(program) / sizeof(int32_t));
    hovm_run(&vm);

    return 0;
}
