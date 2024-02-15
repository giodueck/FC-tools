# target: all - Default target
all:
	gcc fcemu.c FC/overture.c -lSDL2 -lSDL2_ttf -g -o fcemu

# target: release - Build with optimizations and without debug symbols
release:
	gcc fcemu.c FC/overture.c -lSDL2 -lSDL2_ttf -O3 -o fcemu

# target: help - Display available targets
help:
	grep -E "^# target:" Makefile
