// Factorio computer emulator

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>

#include "fcerrors.h"
#include "fcgui.h"
#include "horizon/horizon_compiler.h"
#include "horizon/horizon_parser.h"
#include "horizon/horizon_vm.h"
#include "program.h"

// Error reporting
static char missing_arg = 0;
int error = 0;

// Options
int opt;
extern char *optarg;
extern int optopt;

const char *optstring = ":f:a:bth";
const char *req_opt = "ynnnn";
const char *opt_help[] = {
    "Filename of the program. May be passed without the flag as well",
    "Architecture: currently only horizon is implemented (default: horizon)",
    "Interpret input file as compiled bytecode",
    "Run in TUI instead of GUI",
    "Print this help menu and exit",
};

void help()
{
    int nopt = 0;

    printf("Usage: fcemu [options]\n");
    printf("\nAvailable options:\n");
    for (int i = 0; i < strlen(optstring); i++)
    {
        switch (optstring[i])
        {
        case ':': case '-': case '+': case ';':
            continue;
        default:
            printf("    -%c %s\r\t\t\t%s.\n", optstring[i], (optstring[i+1] == ':' ? "<argument>" : ""), opt_help[nopt]);
            if (req_opt[nopt] == 'y')
            {
                printf("\t\t\tThis option is required.\n");
            }
            nopt++;
            break;
        }
    }
}

static horizon_program_t *ho_program = { 0 };
int parse(FILE *fd, int arch)
{
    if (arch == ARCH_HORIZON)
    {
        ho_program = horizon_parse(fd, NULL, 0);
        if (ho_program->error_count)
        {
            printf("Program contains at least %d errors, exiting\n", ho_program->error_count);
            return ERR_COMPILATION_ERR;
        }
    } else
    {
        printf("Unknown architecture\n");
        return ERR_COMPILATION_ERR;
    }

    return 0;
}

int main(int argc, char **argv)
{
    char filename[BUFSIZ] = { 0 };
    int arch = ARCH_HORIZON;
    int input_binary = 0;
    int tui = 0;

    while ((opt = getopt(argc, argv, optstring)) != -1)
    {
        switch (opt)
        {
        case 'f':
            strncpy(filename, optarg, BUFSIZ - 1);
            break;
        case 'a':
            if (strcmp(optarg, "horizon") == 0)
                arch = ARCH_HORIZON;
            else
            {
                error = ERR_INVALID_ARG;
                help();
                return EXIT_FAILURE;
            }
            break;
        case 'b':
            input_binary = 1;
            break;
        case 't':
            tui = 1;
            break;
        case 'h':
            help();
            return EXIT_SUCCESS;
        case ':':
            error = ERR_NO_ARG;
            help();
            return EXIT_FAILURE;
        default: // '?'
            error = ERR_BAD_OPTION;
            help();
            return EXIT_FAILURE;
        }
    }

    // Error checking
    if (strlen(filename) == 0)
    {
        // optind must be the file to look for
        if (optind < argc)
        {
            strncpy(filename, argv[optind], BUFSIZ - 1);
        }
        else
        {
            error = ERR_MISSING_OPT;
            missing_arg = 'f';
            help();
            return EXIT_FAILURE;
        }
    }
    if (arch == ARCH_OVERTURE)
    {
        printf("Overture programs are not supported for emulation\n");
        return EXIT_FAILURE;
    }

    uint32_t *program = NULL;
    size_t program_size = 0;

    // Parse program if not given binary input
    FILE *fd;
    if (!input_binary)
    {
        if (arch == ARCH_HORIZON)
        {
            if ((fd = fopen(filename, "r")) != NULL)
            {
                int res = parse(fd, arch);
                fclose(fd);
                if (res != 0)
                    return EXIT_FAILURE;

                program_size = ho_program->len_code;
                program = malloc(sizeof(uint32_t) * program_size);
                for (int i = 0; i < program_size; i++)
                    program[i] = ho_program->code[i] & 0xFFFFFFFF;

                horizon_free(ho_program);
            } else
            {
                perror("fcemu");
                return EXIT_FAILURE;
            }
        }
    }
    // Read binary in
    else
    {
        if (arch == ARCH_HORIZON)
        {
            if ((fd = fopen(filename, "rb")) != NULL)
            {
                fseek(fd, 0, SEEK_END);
                program_size = ftell(fd);
                fseek(fd, 0, SEEK_SET);

                program = malloc(sizeof(uint32_t) * program_size);
                fread(program, sizeof(uint32_t), program_size, fd);
            }
        }
    }

    // Run program
    if (program)
    {
        if (tui)
        {
            horizon_vm_t vm = { 0 };

            hovm_load_rom(&vm, program, program_size);
            hovm_run(&vm);

            printf("Time: %u cycles\n", vm.cycles);
            printf("Registers:\n");
            printf("    R0  = %08x = %d\n", vm.registers[HO_R0], vm.registers[HO_R0]);
            printf("    R1  = %08x = %d\n", vm.registers[HO_R1], vm.registers[HO_R1]);
            printf("    R2  = %08x = %d\n", vm.registers[HO_R2], vm.registers[HO_R2]);
            printf("    R3  = %08x = %d\n", vm.registers[HO_R3], vm.registers[HO_R3]);
            printf("    R4  = %08x = %d\n", vm.registers[HO_R4], vm.registers[HO_R4]);
            printf("    R5  = %08x = %d\n", vm.registers[HO_R5], vm.registers[HO_R5]);
            printf("    R6  = %08x = %d\n", vm.registers[HO_R6], vm.registers[HO_R6]);
            printf("    R7  = %08x = %d\n", vm.registers[HO_R7], vm.registers[HO_R7]);
            printf("    R8  = %08x = %d\n", vm.registers[HO_R8], vm.registers[HO_R8]);
            printf("    R9  = %08x = %d\n", vm.registers[HO_R9], vm.registers[HO_R9]);
            printf("    R10 = %08x = %d\n", vm.registers[HO_R10], vm.registers[HO_R10]);
            printf("    R11 = %08x = %d\n", vm.registers[HO_R11], vm.registers[HO_R11]);
            printf("    AR  = %08x = %d\n", vm.registers[HO_AR], vm.registers[HO_AR]);
            printf("    SP  = %08x = %d\n", vm.registers[HO_SP], vm.registers[HO_SP]);
            printf("    LR  = %08x = %d\n", vm.registers[HO_LR], vm.registers[HO_LR]);
            printf("    PC  = %08x = %d\n", vm.registers[HO_PC], vm.registers[HO_PC]);
        }
        else
        {
            // Run emulator in graphical mode
            fcgui_start(arch, program, program_size);
        }
    }


    // Clean up
    if (program)
        free(program);

    return EXIT_SUCCESS;
}
