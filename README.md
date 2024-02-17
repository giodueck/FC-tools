# FC-Tools
Tools for compiling and running programs for my custom computer architectures.

WIP, features may be tested out with Overture for its simplicity and then support Horizon as well. Everest may be skipped entirely as Horizon is similar yet superior.

# Execution
For now, the program is CLI only, the commands are listed with:
```shell
$ ./fctools help
```

Later on, the primary way of running these programs will be with a graphical program built with SDL2, where RAM and register contents and hopefully a screen
will be visible in real time as the program executes.

# Building
Dependencies:\
- SDL2
- SDL2_ttf

Then run
```shell
$ make help
```
for a list of make targets. `make` will compile the program without optimizations and with debug symbols, while `make release` won't.
