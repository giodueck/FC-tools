# FC-Tools
Tools for compiling and running programs for my custom computer architectures.

# Features
This project will slowly be developed and made feature complete.
Currently there exists:
- Overture (POC)
    - [x] Parsing/compiling to internal representation
    - [x] Executing compiled programs
- Horizon
    - [ ] **WIP** Parsing/compiling to internal representation
    - [ ] Compiling to blueprint string
    - [ ] Executing compiled programs
- Graphical emulator
    - [ ] Graphical program to run programs interactively
    - [ ] Step by step debugging
    - [ ] Inspect register and RAM state

# Execution
For now, the program is CLI only, the commands are listed with:
```shell
$ ./fctools help
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
