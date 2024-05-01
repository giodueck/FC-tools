# Horizon ASM version 2
An update on the assembly in [factorio-computing](https://github.com/giodueck/factorio-computing/blob/main/horizon/doc/asm-design.md)

## General considerations
Casing doesn't matter and whitespace (except for newlines) will be ignored. Tokens are
separated by whitespace. One sentence per line, newline character for line endings.

## Registers
There are 12 general-purpose registers named `R0`, `R1`, ..., `R11`.
`AR` is the address register used for load and store operations, `SP` is the stack
pointer, `LR` is the link register, and `PC` is the program counter which
is only writable by jump instructions.

`NIL` is a pseudo-register which is defined as `255` and does not really exist, causing
reads from it to result in `0` and writes to it to be discarded. This can be used to get
a zero or when only the secondary effects of an instruction are desired.

Addressing a non-existent register will cause the same behavior as `NIL` but is considered
an error.

#### The flag register
> This special register cannot be written to or read from directly, it can be set by
> some instructions and is used to evaluate conditional instructions, like `jmp`.

The flags register is set by instructions which have the `s` suffix. The flags are

Name | Description
-----|------------
`Z`  | Result is zero
`N`  | Result is negative
`V`  | Overflow or underflow occurred

`V` is like the carry flag, but all operations are signed anyways so `C` is not needed.

> In Factorio, memory registers can hold several signals at once, meaning the signals can be
> held in 3 different signals, saving several bitwise operations when using and storing them.

## Memory layout
Program instructions and data are all stored in RAM. The layout is as follows:

0. `jmp start` instruction, automatically added to skip data section.
1. Declared data section
2. Program section, instructions
3. Free memory

The free memory space starting address depends on the previous sections, and is pointed to by
the constant `ram_start`. A program can then work with offsets from this address.

Memory is in no way protected, self modifying code, buffer overflows, and executing any memory
address are all possible and are the responsibility of the programmer.

## Immediate values
Immediate values can be used for some instructions and are written as
```
#num
```

For example
```
#10   ; decimal
#0644 ; octal
#0x7F ; hexacedimal
```

The number can be prefixed with `0x` to be interpreted as a hexadecimal number,
or `0` to be interpreted as an octal number.

## Labels
Code labels are constants holding immediate values, and are written as
```
label:
```
and have to be placed in a line by themselves. The value they represent is the address
of the next instruction below the label.

Labels can be used wherever immediate values can, and are invoqued simply by their name:
```
jmp label
```

## Directives
Preprocessor and data section directives were replaced with directives.

### Const
The `@const` directive is now used like so:
```
.const identifier literal
```
where `identifier` is a non-reserved name starting with a letter or underscore and
containing letters, underscores or numbers and no longer than 255 characters, and
`literal` is a numeric literal, in either base 16 when starting with `0x`, base 8
when starting with `0`, or base 10 when starting with any other digit.

### Var
The `.var` directive is used like so:
```
.var identifier value
```
where `identifier` is as defined above, and `value` is either a literal as defined
above or a `.const` identifier.

This identifier then acts as a pointer to a single memory address, which contains the
value.

This directive may only appear before all instructions, as program data is placed
before instruction data.

### Array
The `.array` directive is used like so:
```
.array identifier [ value ] { value-list }
```
where `identifier` is as defined above, `value` is as defined above, and `value-list`
is a comma-separated list of `value`s.

This identifier then acts like a pointer to an array in memory, with `value` items.
The `value-list` may be up to `value` items long, and these are the values the array
is initialized to. If the list is shorter than `value` specifies, the missing values
are assumed to be zero and initialized as such.

This directive may only appear before all instructions, as program data is placed
before instruction data.

### Start
The `.start` directive is used like so:
```
.start
```

This directive signifies the entrypoint of the program, and is implied at the first
instruction if left out.

This directive can be used once in a program.

### Name
The `.name` directive is used like so:
```
.name ; COMMENT
```
where `; COMMENT` is a single line comment. This comment is then used for the
name of the blueprint for the compiled program.

This directive can be used once in a program.

### Desc
The `.desc` directive is used like so:
```
.desc ; MULTI-LINE
      ; COMMENT
```
where a block of lines empty except for comments starting with a comment on the same
line as the directive is set as the description of the blueprint for the compiled
program.

This directive can be used once in a program.

### Macro
The `.macro` directive is used like so:

```
.macro ident literal
. instr1 args1
. instr2 args2
```
where `ident` is an identifier and `literal` is a literal number as defined above or
skipped (implied 0). The macro can have as many instructions as needed, but no
directives or macros. The arguments can be replaced with `$n$` for the instructions
to pass the `n`th macro argument instead of a predefined argument.

The definition of the macro ends after the last instruction prefixed with `.` and
whitespace, and must have at least one instruction.

The builtin macros include:
- `halt`
- `reset`
- `cmp`
- `inc`
- `incs`
- `dec`
- `decs`
- `call`
- `return`
- `mov`
- `movs`
- `mov16`

The `mov16` instruction, as an example, would be defined as
```
; $1$ = Rd
; $2$ = imm16
.macro mov16 2
. push $2$
. pop $1$

```

### Include
**Not implemented**
A directive for the unimplemented textual include concept may be added later.

## Instructions
Syntax is the same as before, the instruction's name followed by its arguments.
The same applies to macros, as they are identical to instructions in their usage.

For a breakdown of the available instructions, see `instructions_machine_code.md`.
