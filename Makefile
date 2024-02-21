# target: all - Default target
all:
	gcc ./*.c overture/*.c horizon/*.c -lSDL2 -lSDL2_ttf -g -o fctools

# target: release - Build with optimizations and without debug symbols
release:
	gcc ./*.c overture/*.c horizon/*.c -lSDL2 -lSDL2_ttf -O3 -o fctools

# target: help - Display available targets
help:
	grep -E "^# target:" Makefile
