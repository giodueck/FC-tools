// Factorio computer compiler

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define DEBUG 1

#include "horizon/horizon_compiler.h"
#include "horizon/horizon_parser.h"
#include "fcerrors.h"
#include "program.h"

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
    "Architecture: overture or horizon (default: horizon)",
    "Generate only raw binary output (.bin output)",
    "Output file name (default: a.out)",
    "Print this help menu and exit",
};

void help()
{
    char curopt = 0;
    int nopt = 0;

    printf("Usage: fctools run [options]\n");
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
        } else
        {
            perror("fcc");
            return EXIT_FAILURE;
        }
    }

    // Compile output
    char binout[BUFSIZ] = { 0 };
    sprintf(binout, "%s.bin", output_filename);
    if (arch == ARCH_HORIZON)
    {
        if ((fd = fopen(binout, "wb")) != NULL)
        {
            fwrite(&ho_program->code, sizeof(uint32_t), ho_program->len_code, fd);

            fclose(fd);
        } else
        {
            perror("fcc");
            return EXIT_FAILURE;
        }
    }

    // Output compiled BP string
    if (arch == ARCH_HORIZON)
    {
        if (!output_binary && ((fd = fopen(output_filename, "w")) != NULL))
        {
            fwrite(&ho_program->code, sizeof(uint32_t), ho_program->len_code, fd);

            fclose(fd);
        } else
        {
            perror("fcc");
            return EXIT_FAILURE;
        }
    }

    // Clean up
    if (arch == ARCH_HORIZON)
        horizon_free(ho_program);

    return EXIT_SUCCESS;
}
