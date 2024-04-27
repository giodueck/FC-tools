#ifndef FC_COMMANDS_H
#define FC_COMMANDS_H

int run(int argc, char **argv);
int compile(int argc, char **argv);
int debug(int argc, char **argv);
int help(int argc, char **argv);

void run_help();
void compile_help();
void debug_help();
void help_help();

#endif // FC_COMMANDS_H
