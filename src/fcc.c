// Factorio computer compiler

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "horizon/horizon_compiler.h"
#include "horizon/horizon_parser.h"
#include "fcerrors.h"
#include "program.h"
#include "bp_creator.h"

// Error reporting
static char missing_arg = 0;
int error = 0;

// Options
int opt;
extern char *optarg;
extern int optopt;

const char *optstring = ":f:a:bo:h";
const char *req_opt = "ynnnn";
const char *opt_help[] = {
    "Filename of the program. May be passed without the flag as well",
    "Architecture: currently only horizon is implemented (default: horizon)",
    "Generate only raw binary output (.bin output)",
    "Output file name (default: a.out)",
    "Print this help menu and exit",
};

void help()
{
    int nopt = 0;

    printf("Usage: fcc [options]\n");
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
    char output_filename[BUFSIZ] = "a.out";
    int arch = ARCH_HORIZON;
    int output_binary = 0;

    while ((opt = getopt(argc, argv, optstring)) != -1)
    {
        switch (opt)
        {
        case 'f':
            strncpy(filename, optarg, BUFSIZ - 1);
            break;
        case 'a':
            if (strcmp(optarg, "overture") == 0)
                arch = ARCH_OVERTURE;
            else if (strcmp(optarg, "horizon") == 0)
                arch = ARCH_HORIZON;
            else
            {
                error = ERR_INVALID_ARG;
                help();
                return EXIT_FAILURE;
            }
            break;
        case 'b':
            output_binary = 1;
            break;
        case 'o':
            strncpy(output_filename, optarg, BUFSIZ - 1);
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

    // Parse program
    FILE *fd;
    if (arch == ARCH_HORIZON)
    {
        if ((fd = fopen(filename, "r")) != NULL)
        {
            if (arch == ARCH_OVERTURE)
            {
                printf("Overture programs are not supported for compilation\n");
                return EXIT_FAILURE;
            }
            int res = parse(fd, arch);
            fclose(fd);
            if (res != 0)
                return EXIT_FAILURE;
        } else
        {
            perror("fcc");
            return EXIT_FAILURE;
        }
    }

    // Output
    if (output_binary)
    {
        char binout[BUFSIZ] = { 0 };
        sprintf(binout, "%s.bin", output_filename);
        if (arch == ARCH_HORIZON)
        {
            if ((fd = fopen(binout, "wb")) != NULL)
            {
                for (int i = 0; i < ho_program->len_code; i++)
                    fwrite(ho_program->code + i, sizeof(uint32_t), 1, fd);

                fclose(fd);
            } else
            {
                perror("fcc");
                return EXIT_FAILURE;
            }
        }
    }
    else
    {
        // Output compiled BP string
        if (arch == ARCH_HORIZON)
        {
            if (((fd = fopen(output_filename, "w")) != NULL))
            {
                int32_t *code_array = malloc(sizeof(int32_t) * ho_program->len_code);
                for (int i = 0; i < ho_program->len_code; i++)
                    code_array[i] = ho_program->code[i] & 0xFFFFFFFF;

                char *bp_str = bp_replace(rom_11_bit, is_rom_11_placeholder, code_array, ho_program->len_code);

                fwrite(bp_str, 1, strlen(bp_str), fd);
                free(code_array);
                free(bp_str);
                fclose(fd);
            } else
            {
                perror("fcc");
                return EXIT_FAILURE;
            }
        }
    }

    // Clean up
    if (arch == ARCH_HORIZON)
        horizon_free(ho_program);

    return EXIT_SUCCESS;
}
