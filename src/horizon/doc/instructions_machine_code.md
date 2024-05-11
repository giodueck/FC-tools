# Horizon instructions and machine language

## Instruction format

1. Opcode:      1 byte
2. Destination: 1 byte
3. Arg 1:       1 byte
4. Arg 2:       1 byte

Busses:
A: Arg1
B: Arg2
R: Result

### Possible meanings of instruction bytes:
- Destination: Always a register
- Arg1: Always a register
- Arg2: Register or imm8, depending on flag in opcode byte

The largest bit in the instruction byte denotes the type of Arg. 2: 0: Reg., 1: Imm8.
For instructions with 1 argument (e.g. conditions, memory and stack) the flag denotes the type
of Arg. 1: 0: Reg., 1: Imm16.

Immediate values must be added using ALU operations, as only the second argument can ever be an immediate.
To facilitate introducing larger immediates, ALU operations to concatenate values exist. Assembly language
could make use of these instructions to abstract away MOVing larger immediates into registers, as MOV itself
will be an abstraction built on ALU operations.

### Asm instruction formats
With all this in mind, the possible instruction formats are:
1. opcode
2. opcode Rd Rn Rm      / opcode Rd Rn      (Rd is result and arg1)
3. opcode Rd Rn imm8    / opcode Rd imm8    (Rd is result and arg1)
4. opcode Rn
5. opcode imm16
6. opcode Rd Rn         / opcode Rd         (Rd is result and arg1)

## Clock stages
1. Load instruction
2. Load registers
3. Save operation result
4. Instruction over

## Registers
Numbered 0x0 to 0xf:
- 0: R0
- ...
- b: R11
- c: RA: Address register
- d: SP
- e: LR
- f: PC

## Instruction breakdown explanation
The instructions are listed by category, each with a most signifficant nibble in the
title.

The instructions are numbered by the opcode, then the instruction in assembly and
finally the formats in which the instruction can accept arguments.

Some instructions also include an explanation, if their function is not obvious from
the name.

## ALU Operations: 0x0

- 00: add
    format: 2, 3
- 01: sub
    format: 2, 3
- 02: mul
    format: 2, 3
- 03: div
    format: 2, 3
- 04: mod
    format: 2, 3
- 05: exp
    format: 2, 3
- 06: lsh
    format: 2, 3
- 07: rsh
    format: 2, 3
- 08: and
    format: 2, 3
- 09: or
    format: 2, 3
- 0a: not
    format: 6
- 0b: xor
    format: 2, 3
- 0c: bcat
    format: 2, 3
    description: (concatenate a byte to the end of a register's value, useful to input longer immediates)
- 0d: hcat
    format: 2, 3
    description: (concatenate a half word to the end of a register)

## ALU Operations with condition flags: 0x1

- 10: adds
    format: 2, 3
- 11: subs
    format: 2, 3
- 12: muls
    format: 2, 3
- 13: divs
    format: 2, 3
- 14: mods
    format: 2, 3
- 15: exps
    format: 2, 3
- 16: lshs
    format: 2, 3
- 17: rshs
    format: 2, 3
- 18: ands
    format: 2, 3
- 19: ors
    format: 2, 3
- 1a: nots
    format: 6
- 1b: xors
    format: 2, 3
- 1c: bcats
    format: 2, 3
    description: (concatenate a byte to the end of a register's value, useful to input longer immediates)
- 1d: hcats
    format: 2, 3
    description: (concatenate a half word to the end of a register)

## Conditions (jumps): 0x2
Instruction formats: 4, 5

- 20: jeq
    assertion: (Z)
- 21: jne
    assertion: (!Z)
- 22: jlt
    assertion: (N != V)
- 23: jgt
    assertion: (!Z & N = V)
- 24: jle
    assertion: (Z & N != V)
- 25: jge
    assertion: (N = V)
- 26: jng
    assertion: (N)
- 27: jpz
    assertion: (!N)
- 28: jvs
    assertion: (V)
- 29: jvc
    assertion: (!V)
- 2a: jmp
    assertion: (always)

## Noop
- 2b: noop
    assertion: (never)
    description: no operation. This instruction is implemented in circuitry as a conditional which asserts false, so it never jumps.

## Memory access: 0x3
Address used is always the one stored in the `AR` register.
The argument is either the source (for store\[id\]) or the destination (for load\[id\])
of the value.

- 30: store
    format: 4, 5
- 31: load
    format: 4
- 32: storei
    format: 4, 5
    description: store and increment AR
- 33: loadi
    format: 4
    description: load and increment AR
- 34: stored
    format: 4, 5
    description: store and decrement AR
- 35: loadd
    format: 4
    description: load and decrement AR

## Stack: 0x3
Instruction formats: 4, 5

The stack is 512 cells deep, and SP always points to the first empty address
on the top of the stack.

The stack is held in an independent block of memory inaccessible by load/store
instructions.

- 8: push
    format: 4, 5
- 9: pop
    format: 4

## Reserved as placeholders 0xFF
For marking placeholder values in empty ROM blueprints, the values 0xFF000000
up to 0xFF00FFFF are reserved.

> In a properly implemented compiler, the placeholders in a blueprint string are only
> read and replaced once, but for cases in which this is not the case, this reservation
> exists.

<!-- ### This is expansion material, not present in the actual processor yet ###-->
<!-- ## Vector operations -->
<!-- ### Registers -->
<!-- Numbered 0x10 to 0x4f -->
<!-- - V0..V15 (operand 1) -->
<!-- - U0..U15 (operand 2) -->
<!-- - T0..T15 (result) -->
<!-- - S0..S15 (result alt) -->
<!---->
<!-- ### Loading and storing -->
<!-- Vx and Ux registers can be written like any other register, and Tx and Sx can be read from, but they are all -->
<!-- one function only. Vx and Ux are write-only and Tx and Sx are read-only for non-vector instructions. -->
<!---->
<!-- ### Vector ALU operations (result T): 0x4 -->
<!-- - 0\*: vtadd -->
<!-- - 1\*: vtsub -->
<!-- - 2\*: vtmul -->
<!-- - 3\*: vtdiv -->
<!-- - 4\*: vtmod -->
<!-- - 5\*: vtexp -->
<!-- - 6\*: vtlsh -->
<!-- - 7\*: vtrsh -->
<!-- - 8\*: vtand -->
<!-- - 9\*: vtor -->
<!-- - a\*: vtnot -->
<!-- - b\*: vtxor -->
<!---->
<!-- ### Vector ALU operations (result S): 0x5 -->
<!-- - 0\*: vsadd -->
<!-- - 1\*: vssub -->
<!-- - 2\*: vsmul -->
<!-- - 3\*: vsdiv -->
<!-- - 4\*: vsmod -->
<!-- - 5\*: vsexp -->
<!-- - 6\*: vslsh -->
<!-- - 7\*: vsrsh -->
<!-- - 8\*: vsand -->
<!-- - 9\*: vsor -->
<!-- - a\*: vsnot -->
<!-- - b\*: vsxor -->
<!---->
<!-- ### Vector register access: 0x6 -->
<!-- - 0: movtv (move registers Tx to Vx) -->
<!-- - 1: movtu (" Tx to Ux) -->
<!-- - 2: movsv (" Sx to Vx) -->
<!-- - 3: movsu (" Sx to Ux) -->
<!-- - 4: movvu (" Vx to Ux) -->
<!-- - 5: movuv (" Ux to Vx) -->
<!-- - 6: copyv (copy single value into all Vx) -->
<!-- - 7: copyu (copy single value into all Vx) -->
