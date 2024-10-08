# General Design

- Circuit decoder, no mircocode
    Is slightly more complex, but much faster
- Harvard architecture, for fast memory access
- Many, useful registers
- Simple, fast, RISC instructions
    Make memory instructions take multiple cycles. This makes instruction cycles shorter in exchange for longer operations taking more cycles.
- Register file inspired by RISC-V
    Registers ordered in an arbitrary way and aliased for the user. E.g. x0 is hardwired zero, and called "zero", x10-x17 are a0-a7.
- Hardware function calls and returns
- Multithreading?
    Worker cores with some protocol, only tricky part is program memory
- Quick memory copying?
- Predication?
    Conditional execution based on flags register
- SIMD?

## Registers
32 registers

### Hardwired zero
Register x0 is always zero on read, and ignores writes.
- ZERO: hardwired zero

### Functional registers
Registers used for some purpose in the hardware
- SP: stack pointer
- RA: return address
- PC: program counter

### Argument registers
For passing arguments and returning results:
- A0-1: function arguments/return values
- A2-7: function arguments

### I/O registers
Communication to input and output registers, to interact with external devices.
Data received and outputted is put into a queue.

- RX: receive
- RS: receive size
- TX: transmit
- CS: chip select

### Temporary
Registers dedicated to temporary use, with aliases in the assembly lang.
- t0-t7: temporaries

### Saved registers
Registers which on return from a function hold the same value as before
- s0-7: saved register

## Instruction format
32 bits
- 8 (31-24): opcode
- 24 (23-0): instruction-specific

## Instructions

### Noop

Opcode 0, does nothing.

### Register operations
Move registers/immediates, test registers, etc.

oooo oooo fddd dd
                 -- ---- ---- ---r rrrr
                 ii iiii iiii iiii iiii

- 1 (23): flags updated
- 5 (22-18): destination register

#### Register
- 5 (4-0): source register

#### Immediate
- 18 (17-0): immediate (signed 18-bit)

### Arithmetic
Arithmetic and bitwise operations.

oooo oooo fddd dd
                 ii iiii iiii jjjj jjjj
                 -- ---r rrrr jjjj jjjj
                 -- ---r rrrr ---s ssss

- 1 (23): flags updated
- 5 (22-18): destination register

#### Imm-Imm
- 10 (17-8): immediate 1 (signed 10-bit)
- 8 (7-0): immediate 2 (signed 8-bit)

#### Reg-Imm
- 5 (12-8): register operand 1
- 8 (7-0): immediate operand 2 (signed 8-bit)

#### Reg-Reg
- 5 (12-8): operand 1
- 5 (4-0): operand 2

### Jumps and subroutines
Jumps, branching and subroutine calls. Jumps just send the link to x0.

oooo oooo -ddd dd
                 -- iiii iiii iiii iiii
                 -- ---- ---- ---r rrrr

- 5 (22-18): link register

#### Immediate
- 16 (15-0): immediate (unsigned 16-bit)

#### Register
- 5 (4-0): register

### Memory access
Store and load.

Store:
oooo oooo -sss ss
                 -- iiii iiii iiii iiii
                 -- ---- ---- ---r rrrr

Load:
oooo oooo -ddd dd
                 -- iiii iiii iiii iiii
                 -- ---- ---- ---r rrrr

- 1 (23): flags updated
- 5 (22-18): source/destination register

#### Immediate
- 16 (15-0): immediate address (unsigned 16-bit)

#### Register
- 5 (4-0): register address

### Stack access

Push:
oooo oooo
          ---- --ii iiii iiii iiii iiii
          ---- ---- ---- ---- ---r rrrr

Pop:
oooo oooo fddd dd-- ---- ---- ---- ----

- 1 (23): flags updated
- 5 (22-18): destination register

#### Immediate
- 18 (17-0): immediate (signed 18-bit)

#### Register
- 5 (4-0): register

### ROM to RAM copy

Special mode of execution in which words read from ROM are transferred to RAM instead of interpreted as instructions.

The instruction encodes the number of words following it to transfer to an address stored in a register. The address is incremented for each new word, such that the result is the same sequence of words in a contiguous array of RAM starting at the address specified by the register argument.

A similar instruction does not exist for copying a single word to an address or a register, as the 2 cycles needed to complete it are not much faster than the 3 with basic arithmetic operations or 2 with cleverer arithmetic ops.

oooo oooo -rrr rr-- iiii iiii iiii iiii

- 5 (22-18): register (starting address)
- 16 (15-0): number of words (unsigned 16-bit)

### Reserved
Opcode 0xFF will be reserved for placeholders in ROM blueprints.

## Memory Access
Memory access, with my current designs, takes a long time compared to other operations. To make the cycles shorter, it will be divided in two suboperations in machine code:

1. Load memory address and value and do post-operation on register
2. Load value into register

For store, the second step could be ommitted
