# target: all - Default target
all:
	gcc src/fccommands.c src/fctools.c src/helpers.c src/overture/*.c src/horizon/*.c -lSDL2 -lSDL2_ttf -lz -lm -g -o fctools
	gcc src/fcc.c src/horizon/horizon_parser.c src/horizon/horizon_compiler.c src/helpers.c src/bp_creator.c src/rom_bp_strings.c -lz -g -o fcc -Wall
	gcc src/fcemu.c src/horizon/horizon_parser.c src/horizon/horizon_compiler.c src/horizon/horizon_vm.c src/helpers.c -lm -lz -g -o fcemu -Wall


# target: release - Build with optimizations and without debug symbols
release:
	gcc src/fccommands.c src/fctools.c src/helpers.c src/overture/*.c src/horizon/*.c -lSDL2 -lSDL2_ttf -lz -lm -O3 -o fctools
	gcc src/fcc.c src/horizon/horizon_parser.c src/horizon/horizon_compiler.c src/helpers.c src/bp_creator.c src/rom_bp_strings.c -lz -O3 -o fcc
	gcc src/fcemu.c src/horizon/horizon_parser.c src/horizon/horizon_compiler.c src/horizon/horizon_vm.c src/helpers.c -lm -lz -O3 -o fcemu -Wall

# target: help - Display available targets
help:
	grep -E "^# target:" Makefile
