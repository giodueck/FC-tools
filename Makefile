# target: all - Default target
all:
	gcc fccommands.c fctools.c helpers.c overture/*.c horizon/*.c -lSDL2 -lSDL2_ttf -g -o fctools
	gcc fcc.c horizon/*_parser.c horizon/*_compiler.c helpers.c -g -o fcc

# target: release - Build with optimizations and without debug symbols
release:
	gcc fccommands.c fctools.c helpers.c overture/*.c horizon/*.c -lSDL2 -lSDL2_ttf -O3 -o fctools
	gcc fcc.c horizon/*_parser.c horizon/*_compiler.c helpers.c -O3 -o fcc

# target: help - Display available targets
help:
	grep -E "^# target:" Makefile
