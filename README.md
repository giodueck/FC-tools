# FC-Tools
Tools for compiling and running programs for my custom computer architectures.

# Features
This project will slowly be developed and made feature complete.
Currently there exists:
- Overture (POC)
    - [x] Parsing/compiling to internal representation
    - [x] Executing compiled programs
- Horizon
    - [x] Parsing/compiling to internal/binary representation
    - [x] Compiling to blueprint string
    - [ ] **WIP** Executing compiled programs
- Graphical emulator
    - [ ] Graphical program to run programs interactively
    - [ ] Step by step debugging
    - [ ] Inspect register and RAM state

# Execution
For now, the program is CLI only, the commands are listed with:
```shell
$ ./fctools help
```
> Note:
> fctools as a single binary is deprecated and will be removed soon. The idea of one complicated binary
> with several commands didn't sit right with me. Instead, a compiler and a runner will be available
> separately. Unix philosophy and whatnot

To compile a Horizon program use `fcc`:
```shell
$ ./fcc -h
```

# Building
Dependencies:
- SDL2
- SDL2_ttf

Then run
```shell
$ make help
```
for a list of make targets. `make` will compile the program without optimizations and with debug symbols, while `make release` won't.
