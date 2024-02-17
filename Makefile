# target: all - Default target
all:
	gcc fctools.c fccommands.c FC/overture.c FC/overture_compiler.c -lSDL2 -lSDL2_ttf -g -o fctools

# target: release - Build with optimizations and without debug symbols
release:
	gcc fctools.c fccommands.c FC/overture.c FC/overture_compiler.c -lSDL2 -lSDL2_ttf -O3 -o fctools

# target: help - Display available targets
help:
	grep -E "^# target:" Makefile
