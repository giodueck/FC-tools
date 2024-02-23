#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "horizon_compiler.h"

typedef struct Node {
    char **tokens;
    int freeable;
    struct Node *next;
} token_node;

// Internal functions

// Create liked list of n nodes
static token_node *create_tokenll(int n)
{
    token_node *head = malloc(sizeof(token_node));
    token_node *tail = head;
    for (int i = 0; i < n - 1; i++)
    {
        tail->next = malloc(sizeof(token_node));
        tail->freeable = 0;
        if (tail->next == NULL)
        {
            perror("create_tokenll");
            return head;
        }
        tail = tail->next;
    }
    return head;
}

// Free linked list, and if any node has a non-null tokens element and the freeable flag
// is set, then free the tokens too
static void free_tokenll(token_node *head)
{
    token_node *prev_head;

    while (head)
    {
        if (head->tokens != NULL && head->freeable != 0)
            free(head->tokens);

        prev_head = head;
        head = head->next;
        free(prev_head);
    }
}

static void insert_token_after_node(token_node *head, token_node *new_node)
{
    new_node->next = head->next;
    head->next = new_node;
}

/* locate_forward, locate_backward and str_replace taken from https://stackoverflow.com/a/12546318 */

// locate_forward: compare needle_ptr and read_ptr to see if a match occured
// needle_ptr is updated as appropriate for the next call
// return 1 if match occured, false otherwise
static inline int locate_forward(char **needle_ptr, char *read_ptr, const char *needle, const char *needle_last)
{
    if (**needle_ptr == *read_ptr) {
        (*needle_ptr)++;
        if (*needle_ptr > needle_last) {
            *needle_ptr = (char *)needle;
            return 1;
        }
    }
    else
        *needle_ptr = (char *)needle;
    return 0;
}

// locate_backward: compare needle_ptr and read_ptr to see if a match occured
// needle_ptr is updated as appropriate for the next call
// return 1 if match occured, 0 otherwise
static inline int locate_backward(char **needle_ptr, char *read_ptr, const char *needle, const char *needle_last)
{
    if (**needle_ptr == *read_ptr) {
        (*needle_ptr)--;
        if (*needle_ptr < needle) {
            *needle_ptr = (char *)needle_last;
            return 1;
        }
    }
    else
        *needle_ptr = (char *)needle_last;
    return 0;
}

// str_replace(haystack, haystacksize, oldneedle, newneedle) --
//  Search haystack and replace all occurences of oldneedle with newneedle.
//  Resulting haystack contains no more than haystacksize characters (including the '\0').
//  If haystacksize is too small to make the replacements, do not modify haystack at all.
//
// RETURN VALUES
// str_replace() returns haystack on success and NULL on failure. 
// Failure means there was not enough room to replace all occurences of oldneedle.
// Success is returned otherwise, even if no replacement is made.
static char *str_replace(char *haystack, size_t haystacksize, const char *oldneedle, const char *newneedle)
{
    size_t oldneedle_len = strlen(oldneedle);
    size_t newneedle_len = strlen(newneedle);
    char *oldneedle_ptr;    // locates occurences of oldneedle
    char *read_ptr;         // where to read in the haystack
    char *write_ptr;        // where to write in the haystack
    const char *oldneedle_last =  // the last character in oldneedle
        oldneedle +
        oldneedle_len - 1;

    // Case 0: oldneedle is empty
    if (oldneedle_len == 0)
        return haystack;     // nothing to do; define as success

    // Case 1: newneedle is not longer than oldneedle
    if (newneedle_len <= oldneedle_len) {
        // Pass 1: Perform copy/replace using read_ptr and write_ptr
        for (oldneedle_ptr = (char *)oldneedle,
            read_ptr = haystack, write_ptr = haystack;
            *read_ptr != '\0';
            read_ptr++, write_ptr++)
        {
            *write_ptr = *read_ptr;
            int found = locate_forward(&oldneedle_ptr, read_ptr,
                        oldneedle, oldneedle_last);
            if (found)  {
                // then perform update
                write_ptr -= oldneedle_len;
                memcpy(write_ptr+1, newneedle, newneedle_len);
                write_ptr += newneedle_len;
            }
        }
        *write_ptr = '\0';
        return haystack;
    }

    // Case 2: newneedle is longer than oldneedle
    else {
        size_t diff_len =       // the amount of extra space needed
            newneedle_len -     // to replace oldneedle with newneedle
            oldneedle_len;      // in the expanded haystack

        // Pass 1: Perform forward scan, updating write_ptr along the way
        for (oldneedle_ptr = (char *)oldneedle, read_ptr = haystack, write_ptr = haystack;
            *read_ptr != '\0' && write_ptr < haystack + haystacksize;
            read_ptr++, write_ptr++)
        {
            int found = locate_forward(&oldneedle_ptr, read_ptr,
                        oldneedle, oldneedle_last);
            if (found) {
                // then advance write_ptr
                write_ptr += diff_len;
            }
        }
        if (write_ptr >= haystack+haystacksize)
            return NULL; // no more room in haystack

        // Pass 2: Walk backwards through haystack, performing copy/replace
        for (oldneedle_ptr = (char *)oldneedle_last;
            write_ptr >= haystack;
            write_ptr--, read_ptr--)
        {
            *write_ptr = *read_ptr;
            int found = locate_backward(&oldneedle_ptr, read_ptr,
                        oldneedle, oldneedle_last);
            if (found) {
                // then perform replacement
                write_ptr -= diff_len;
                memcpy(write_ptr, newneedle, newneedle_len);
            }
        }
        return haystack;
    }
}

// Note: This function returns a pointer to a substring of the original string.
// If the given string was allocated dynamically, the caller must not overwrite
// that pointer with the returned value, since the original pointer must be
// deallocated using the same allocator with which it was allocated.  The return
// value must NOT be deallocated using free() etc.
static char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

static void horizon_syntax_error(const char *message, int line_minus_one)
{
    printf("Error on line %d:\n\t%s\n", line_minus_one + 1, message);
}

// End internal functions


#define SECTION_PROGRAM 0
#define SECTION_MACRO   1
#define SECTION_DATA    2

const char *instr_names[] = {
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "MOD",
    "EXP",
    "LSH",
    "RSH",
    "AND",
    "OR",
    "NOT",
    "XOR",
    "BCAT",
    "HCAT",
    "ADDS",
    "SUBS",
    "MULS",
    "DIVS",
    "MODS",
    "EXPS",
    "LSHS",
    "RSHS",
    "ANDS",
    "ORS",
    "NOTS",
    "XORS",
    "BCATS",
    "HCATS",
    "JEQ",
    "JNE",
    "JLT",
    "JGT",
    "JLE",
    "JGE",
    "JNG",
    "JPZ",
    "JVS",
    "JVC",
    "JMP",
    "NOOP",
    "STORE",
    "LOAD",
    "STOREI",
    "LOADI",
    "STORED",
    "LOADD",
    "PUSH",
    "POP",
};

enum instr_opcode {
    ADD =       0,
    SUB =       1,
    MUL =       2,
    DIV =       3,
    MOD =       4,
    EXP =       5,
    LSH =       6,
    RSH =       7,
    AND =       8,
    OR =        9,
    NOT =       10,
    XOR =       11,
    BCAT =      12,
    HCAT =      13,
    ADDS =      16,
    SUBS =      17,
    MULS =      18,
    DIVS =      19,
    MODS =      20,
    EXPS =      21,
    LSHS =      22,
    RSHS =      23,
    ANDS =      24,
    ORS =       25,
    NOTS =      26,
    XORS =      27,
    BCATS =     28,
    HCATS =     29,
    JEQ =       32,
    JNE =       33,
    JLT =       34,
    JGT =       35,
    JLE =       36,
    JGE =       37,
    JNG =       38,
    JPZ =       39,
    JVS =       40,
    JVC =       41,
    JMP =       42,
    NOOP =      43,
    STORE =     48,
    LOAD =      49,
    STOREI =    50,
    LOADI =     51,
    STORED =    52,
    LOADD =     53,
    PUSH =      56,
    POP =       57
};

const char *builtin_macro_names[] = {
    "HALT",
    "RESET",
    "CMP",
    "INC",
    "INCS",
    "DEC",
    "DECS",
    "CALL",
    "RETURN",
    "MOV",
    "MOVS",
    "MOV16"
};

const char *preprocessor_reserved[] = {
    "CONST",
    "DEFINE",
    "INCLUDE",
    "REP",
    "END"
};

const char *data_directive_names[] = {
    "TIMES"
};

const char *register_names[] = {
    "R0",
    "R1",
    "R2",
    "R3",
    "R4",
    "R5",
    "R6",
    "R7",
    "R8",
    "R9",
    "R10",
    "R11",
    "AR",
    "SP",
    "LR",
    "PC",
    "NIL"
};

enum register_value {
    R0 =    0,
    R1 =    1,
    R2 =    2,
    R3 =    3,
    R4 =    4,
    R5 =    5,
    R6 =    6,
    R7 =    7,
    R8 =    8,
    R9 =    9,
    R10 =   10,
    R11 =   11,
    AR =    12,
    SP =    13,
    LR =    14,
    PC =    15,
    NIL =   255
};

const char *reserved_labels[] = {
    "RAM_START"
};

int horizon_is_reserved(const char *token)
{
    for (int i = 0; i < sizeof(instr_names) / sizeof(char*); i++)
        if (strcmp(token, instr_names[i]) == 0)
            return 1;

    for (int i = 0; i < sizeof(builtin_macro_names)/sizeof(char*); i++)
        if (strcmp(token, builtin_macro_names[i]) == 0)
            return 1;

    for (int i = 0; i < sizeof(preprocessor_reserved)/sizeof(char*); i++)
        if (strcmp(token, preprocessor_reserved[i]) == 0)
            return 1;

    for (int i = 0; i < sizeof(data_directive_names)/sizeof(char*); i++)
        if (strcmp(token, data_directive_names[i]) == 0)
            return 1;

    for (int i = 0; i < sizeof(register_names)/sizeof(char*); i++)
        if (strcmp(token, register_names[i]) == 0)
            return 1;

    for (int i = 0; i < sizeof(reserved_labels)/sizeof(char*); i++)
        if (strcmp(token, reserved_labels[i]) == 0)
            return 1;

    return 0;
}

int horizon_symbol_exists(program_t program, const char *token)
{
    for (int i = 0; i < program.len_symbols; i++)
    {
        if (strcmp(token, program.symbols[i].name) == 0)
            return 1;
    }
    return 0;
}

void horizon_handle_const(program_t *program, char **tokens, int i)
{
    if (tokens[1] == NULL || tokens[2] == NULL)
    {
        horizon_syntax_error("@CONST must be followed by an identifier and a value", i);
        program->error_count++;
        return;
    }

    // check if valid identifier
    if (horizon_is_reserved(tokens[1]))
    {
        horizon_syntax_error("@CONST identifiers cannot be reserved words", i);
        program->error_count++;
        return;
    }

    // check if symbol exists
    if (!horizon_symbol_exists(*program, tokens[1]))
    {
        uint32_t val;
        if (tokens[2][0] == '0')
        {
            if (tokens[2][1] == 'X')
            {
                char *end = NULL;
                val = strtol(tokens[2], &end, 16);
                // end should be the end of the string
                if (end[0] != '\0')
                {
                    horizon_syntax_error("Invalid hex literal", i);
                    program->error_count++;
                    return;
                }
            } else if (tokens[2][1] == 'B')
            {
                char *end = NULL;
                val = strtol(&tokens[2][2], &end, 2);
                // end should be the end of the string
                if (end[0] != '\0')
                {
                    horizon_syntax_error("Invalid binary literal", i);
                    program->error_count++;
                    return;
                }
            } else
            {
                char *end = NULL;
                val = strtol(tokens[2], &end, 8);
                // end should be the end of the string
                if (end[0] != '\0')
                {
                    horizon_syntax_error("Invalid octal literal", i);
                    program->error_count++;
                    return;
                }
            }
        } else
        {
            char *end = NULL;
            val = strtol(tokens[2], &end, 10);
            // end should be the end of the string
            if (end[0] != '\0')
            {
                horizon_syntax_error("Invalid literal", i);
                program->error_count++;
                return;
            }
        }

        program->symbols[program->len_symbols++] = (symbol_t) { .name = tokens[1], .value = val };
    }
    // else if tokens[1] is already a symbol
    else
    {
        char buf[BUFSIZ] = "";
        snprintf(buf, BUFSIZ, "%s defined multiple times", tokens[1]);
        horizon_syntax_error(buf, i);
        program->error_count++;
        return;
    }
}

// Parse program to build the symbol table and check for errors
// Returns the pointer to a newly allocated program_t
// If there are any errors, they will be added to the err_array, as many as
// err_array_size allows. The program_t return will have the total error count.
// After running horizon_parse, run horizon_free on the pointer to free its
// allocated memory, regardless of if the program was error-free or not.
program_t *horizon_parse(FILE *fd, error_t *err_array, int err_array_size)
{
    fseek(fd, 0, SEEK_END);
    int size = ftell(fd);
    rewind(fd);

    program_t program = { ARCH_HORIZON };
    char *program_buf = malloc(size + 1);
    char *buf = malloc(size + 1);

    program.lines_buf = program_buf;
    size_t n_lines = 1000;
    program.lines = malloc(sizeof(char*) * n_lines);

    // Read whole program in
    int i = 0;
    program.len_lines = 0;
    while (!feof(fd))
    {
        // Read a line
        int c;
        int j = 0;
        while ((c = getc(fd)) != '\n' && c != EOF)
        {
            buf[j++] = toupper(c);
            i++;

            // Cut off comments, but keep reading the line
            if (c == ';')
                buf[j - 1] = '\0';

            // TODO: Name of program, description of program
        }
        buf[j] = 0;

        if (program.len_lines >= n_lines)
        {
            n_lines += 1000;
            program.lines = realloc(program.lines, sizeof(char*) * n_lines);
        }
        strcpy(&program_buf[i - j], buf);
        program.lines[program.len_lines++] = trimwhitespace(&program_buf[i - j]);
        i++;
    }

    free(buf);

    // Allocate the needed program space
    program.line_executable = malloc(sizeof(int) * program.len_lines);
    int symbol_space = 100;
    program.symbols = malloc(sizeof(symbol_t) * symbol_space);

    // First Pass: tokenize, get section, consts, handle built-in macros and @REP
    // blocks, handle labels, check instructions

    // Pointer to array of strings
    const int max_tokens = 128;
    char ***tokens = malloc(sizeof(char**) * program.len_lines);
    for (int i = 0; i < program.len_lines; i++)
    {
        // Just allocate a lot of space, normal instructions don't have a lot of
        // tokens, but macros may
        tokens[i] = malloc(sizeof(char*) * max_tokens);

        for (int j = 0; j < max_tokens; j++)
        {
            if (j == 0)
                tokens[i][j] = strtok(program.lines[i], " \t,");
            else
                tokens[i][j] = strtok(NULL, " \t,");

            if (tokens[i][j] == NULL)
                break;
            program.line_executable[i] = 1;
        }
    }

    // Iterate through lines for first pass
    int repeat = 0;
    int section = SECTION_PROGRAM;
    int program_reached = 0;
    for (int i = 0; i < program.len_lines; i++)
    {
        if (tokens[i][0] == NULL)
            continue;

        // TODO: handle REP blocks

        // Preprocessor
        if (tokens[i][0][0] == '@')
        {
            if (strcmp(tokens[i][0], "@CONST") == 0)
            {
                if (program.len_symbols >= symbol_space - 1)
                {
                    symbol_space += 100;
                    program.symbols = realloc(program.symbols, sizeof(symbol_t) * symbol_space);
                }
                horizon_handle_const(&program, tokens[i], i);
            }
        }
    }

    // End first pass
    if (program.error_count)
        printf("%d error%s.\n\n", program.error_count, (program.error_count > 1) ? "s" : "");
    printf("Symbols:\n");
    for (int i = 0; i < program.len_symbols; i++)
    {
        printf("\t%s: %d\n", program.symbols[i].name, program.symbols[i].value);
    }

    // Success
    for (int i = 0; i < program.len_lines; i++)
        free(tokens[i]);
    free(tokens);

    program_t *ret = malloc(sizeof(program_t));
    *ret = program;
    return ret;
}

// Frees memory allocated by horizon_parse
void horizon_free(program_t *program)
{
    if (!program)
        return;
    if (program->lines_buf)
        free(program->lines_buf);
    if (program->lines)
        free(program->lines);
    if (program->line_executable)
        free(program->line_executable);
    if (program->code)
        free(program->code);
    free(program);
    return;
}

// Parse the program at the given src_filepath and save the result in the given
// destination path dst_filename.
// The options struct changes the way the procedure operates
void horizon_compile(const char *dst_filename, const char *src_filename, struct horizon_compiler_opt *options)
{
    horizon_perror(HOR_ERR_NOT_IMPLEMENTED);
    return;
}

// Print formatted error message to the console
void horizon_perror(int error)
{
    printf("Error: ");

    switch (error)
    {
        // No error
        case HOR_OK:
            printf("\r       \r");
            break;

        // Implementation
        case HOR_ERR_NOT_IMPLEMENTED:
            printf("not implemented\n");
            fflush(stdout);
            break;

        // Parsing

        // Compiling

        // Runtime
        default:
            printf("unknown error code\n");
    }
}
