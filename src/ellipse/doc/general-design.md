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
- PC: program counter
- SP: stack pointer
- RA: return address

### Argument registers
For passing arguments and returning results:
- A0-1: function arguments/return values
- A2-7: function arguments

### I/O registers
Communication to input and output registers, to interact with external devices.
Data received and outputted is put into a queue.

- RX: receive: read received word
- RC: receive count: how many words are in the receive queue
- TX: transmit: add word to transmit queue
- TC: transmit count: how many words are in the transmit queue to be read by the receiver

### Temporary
Registers dedicated to temporary use, with aliases in the assembly lang.
- t0-t5: temporaries

### Saved registers
Registers which on return from a function hold the same value as before
- s0-7: saved register

### Misc registers
Registers with miscellaneous special functions
- PRNG: pseudo-random number, generated each time it is read, set seed by writing
- TICK: self-incrementing counter, increments every cycle

## Instruction format
32 bits
- 8 (31-24): opcode
- 24 (23-0): instruction-specific

## Instructions

### Noop

Waste a cycle, do nothing.

Mov ZERO to ZERO

### Register operations
Move registers/immediates, test registers, etc.

oooo oooo fddd dd--
                    ---- ---- ---s ssss
                    iiii iiii iiii iiii

- 1 (23): flags updated
- 5 (22-18): destination register

#### Register
- 5 (4-0): source register

#### Immediate
- 16 (15-0): immediate (signed 16-bit)

### Arithmetic
Arithmetic and bitwise operations.

oooo oooo fddd dda
                  r rrrr iiii iiii iiii
                  r rrrr ---- ---s ssss

- 1 (23): flags updated
- 5 (22-18): destination register
- 1 (17): operands reversed

#### Reg-Imm
- 5 (16-12): register operand 1
- 12 (11-0): immediate operand 2 (signed 12-bit)

#### Reg-Reg
- 5 (16-12): register operand 1
- 5 (4-0): register operand 2

### Jumps and subroutines
Jumps, branching and subroutine calls. Jumps just send the link to x0.

oooo oooo l--- --
                 -- iiii iiii iiii iiii
                 -- ---- ---- ---r rrrr

- 23 (23): link, store PC+1 in RA

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
oooo oooo fddd dd
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
          ---- ---- iiii iiii iiii iiii
          ---- ---- ---- ---- ---r rrrr

Pop:
oooo oooo fddd dd-- ---- ---- ---- ----

- 1 (23): flags updated
- 5 (22-18): destination register

#### Immediate
- 16 (15-0): immediate (signed 16-bit)

#### Register
- 5 (4-0): register

### ROM to RAM copy

Special mode of execution in which words read from ROM are transferred to RAM instead of interpreted as instructions.

The instruction encodes the number of words following it to transfer to an address stored in a register. The address is incremented for each new word, such that the result is the same sequence of words in a contiguous array of RAM starting at the address specified by the register argument.

The address is stored in a special-purpose register, the original register is not incremented.

A similar instruction does not exist for copying a single word to a register, as the 2 cycles needed to complete it are not faster than the 2 with MOV and MOVH

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

### Rx/Tx
Special registers allow sending and receiving data through registers to peripherals.

A double stack is used to simulate a queue:
- A receiver stack accepts input data
- An output stack offers output data
- An instruction to flush to output clears the output stack and pushes the reversed input stack

Special register RX is read-only, writing has no effect. Reads produce the top element from the queue without popping, or zero if the queue is empty.

Special register TX is write-only, reading produces a zero. Writes push data onto the input stack of the Tx queue, and if the maximum capacity is reached, further writes are discarded.

Flushing the Tx input to output or popping from the Tx or Tx stack is done through instructions. While a stack is being flushed from input to output, any operations to RX and TX are discarded.

Flushing the Rx input stack so that the data is readable from RX is the job of the peripheral. The amount of data in each stack can be read from RC and TC, for received words yet to pop and pushed words yet to transmit.

Poprx, Poptx, Flushtx:
oooo oooo ---- ---- ---- ---- ---- ----
