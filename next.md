Name TBD
- Eclipse
- Atlantis

# Core ideas
- Pipelined design:
  - Fetch, decode and read registers, execute/memory access, writeback
  - Writeback early enough that a decode can read the same register, WB on first half, read on latter half of cycle
- Conditional execution of all instructions, like ARM
- Simple decoding, like RISC-V
- Small memory space for faster and more compact designs
- Rom+Ram Harvard architecture
- Short cycles: target 3-4 ticks. Maybe introduce stalls for certain instructions only
  - *Post implementation*: actually 6 ticks

# Stage sketches
Clock pulses one signal on stage start and once in the middle of the stage for the decode stage.

## Fetch
Rom with CCs, a decider with condition $A_s \le A \le A_e$, i.e. two conditions, to avoid splitting address into upper and lower half first and save a cycle. This output into a decoder which does take $A \mod C_s$, where $C_s$ is the amount of signals in one CC.

Est. Time (each bullet is one cycle)
- DC (read CC) + AC (address modulo)
- DC (filter CC content with address modulo)

## Decode
Any section of a bitfield can be extracted in 2 ticks with AND -> RSH.

Detect instruction format in 2 ticks as well: AND -> compare. This allows saving into control signal buffer when the rest of the decode is done.

Read registers in ticks 3-4, when clock pulses for the second time.

## Execute/memory access
Support all natively supported arithmetic operations.

Evaluate conditions and branching here. Pass NOP to WB is false 

Handle branches here:
1. Update PC
2. Optionally set LR=oldPC+1 in WB
3. Override condition codes for 2 cycles to flush pipeline

Calculate flags, but not as complex as ARM.

> Set conditions at end of stage, so the next cycle they can be read.

Memory access is with immediate or register address, no calculations supported. Value for store is always a register

Memory cells:
- 1 tick design from forums
- Input/output take into account a load control signal and an address range, similar to Rom

Load:
- Addr -> Output DC, Addr -> Modulo range
- Output -> Filter with modulo

Store:
- Addr -> Input DC, Addr -> Modulo range
- Filter cell contents, insert value with modulo (maybe takes 2 ticks)

Support address register post increment/decrement in writeback stage (if I find a way to save two values at once)

## Writeback
Result is written to destination register as soon as possible, before the read in the decode stage.

# Registers
4-bit address: x0 to x15.
Special purpose:
- x0: reads always result in 0, writes are ignored.
- x15: PC: will always be a few instructions ahead of the currently executed one, due to the fetch stage
- x14: LR
- x13: SP
- x11, x12: timers, tick down at 60 per second until reaching 0

General purpose:
- x1-x4: argument/volatile registers
- x5-x10: saved registers (must be saved before use in subroutines)

Flags are indirect: either an instruction sets them or a special instruction sets the desired flags.

## Pipeline considerations
### PC
Must be incremented independently of writeback.

Writeback to PC invalidates the next 2 instructions in the pipeline: the ones in the fetch and decode stages.

If the 0 instruction is arithmetic, operands and result will be x0 and flags will be ignored. This is then a NOP and to invalidate instructions simply pass 0 to the execute stage twice instead of the instructions in the pipeline to be flushed.

### Address writeback
Experiment with writing a different value at the same time, since deciders can decide from which wire color to read. Depends on the design of the memory cell.

If this is possible, branch eith link can also easily be done.

If it is not possible/slow, translate into separate instruction on compilation:
- stri/ldri: ldr/str, addi/subi 1
- bl: subi lr pc 1, b

#### Brainstorming
Register write with:
- If W (color) = xN => Save M (color)

# ISA
All instructions should have a very similar schema for getting operators and destinations to simplify and thus accelerate decoding.

## Format
```
cccc oooo o00f dddd xxxx yyyy eeee eeee
cccc oooo o01f dddd xxxx iiii iiii iiii
cccc oooo o10f dddd aaaa aaaa aaaa aaaa
cccc oooo o11f llll llll llll llll llll
```

> [!Idea] Risc-V places all immediates with their sign at bit 31 to simplify sign extension, and extends all immediates. Condition codes could be placed at bits 0:3 just as well

```
eeee eeee ssss rrrr dddd oooo o00f cccc
iiii iiii iiii rrrr dddd oooo o01f cccc
aaaa aaaa aaaa aaaa dddd oooo o10f cccc
llll llll llll llll llll oooo o11f cccc
```

Legend:
- c: condition code bits
- o: opcode bits
- d: destination/source register
- r: operand 1
- s: operand 2
- i: (signed) immediate $[-2048, 2047]$
- f: flags enable/branch with link
- a: (signed) immediate $[-32768, 32767]$
- l: (signed) immediate $[-524288, 524287]$
- e: function specific

## Condition codes
Flags:
- Zero
- Negative
- Overflow
  - Addition: two operands of same sign result in different sign
  - Subtraction: operands of different sign result in sign of second operand
- Error: divide by zero

Undefined is always false.

0. Always
1. Equal (Z)
2. Not equal (!Z)
3. Negative (N)
4. Positive or zero (!N)
5. Less than (N != V)
6. Less than or equal (Z or N != V)
7. Greater than (!Z and N = V)
8. Greater than or equal (N = V)
9. Overflow set (V)
10. Overflow clear (!V)
11. Error set (E)
12. Error clear (E)

## Instructions
Arithmetic: (format 0 or 1 or 2)
1. Add
2. Sub
3. Subn (swap operands)
4. Mul
5. Div
6. Mod
7. Exp
8. Lsh/Rotl (if operand 2 is >= 32, a rotate happens)
9. Rsh/Rotr
10. And
11. Or
12. Xor

> [!Note] Format 1 and 2 have immediates as operand 2, format 2 has x0 as implicit operand 1.
> All non-specified registers are assumed to be x0, the only exception is formats with immediates, where the S register is ignored and the immediate becomes operand 2.
> This makes format 3 almost useless, except to set some flags.

Opcodes: 0-11, 0x0-0xb

Memory: (format 2)
1. Load
2. Store
3. Loadi
4. Storei
5. Loadd
6. Stored
7. Load signed imm16 into upper halfword (31:16)

> [!Note] The memory address is always operand 2, meaning an immediate or the S register for format 0. The source/destination register is the D register.

Opcodes: 16-22, 0x10-0x16

PPU: (format TBD)
Opcodes: 24-27, 0x18-0x1b

Branch: (format 2 or 3)
1. Branch: register address if format <= 2, immediate if format 3

> [!Note] All writes to PC constitute a branch
> What this instruction does is add an offset to the PC, instead of adding 1 at the end of the cycle.
> Writes to PC take an additional cycle for the register to be written, which means this instruction is 2 cycles faster, since the offset is added at the execute stage. It avoids waiting one extra cycle, which then also avoids needing to flush the pipeline one cycle longer.
> If the F flag is set, store PC+1 in LR. This is done in the writeback cycle, while the pipeline is still flushing, so no performance penalty here.

Opcodes: 28, 0x1c

Control:
1. Set flags: bits `znve`
2. Break: stop the clock until manual action is taken
3. Wait: stop the clock until the given register hits 0. If a counter is given, wait until it ticks down, if not and the register is not 0, wait indefinitely

Opcodes: 29-31, 0x1d-0x1f

> [!Note] Since some encodings remain unused due to the format requirements, the same opcodes may be repurposed with other format bits for other instructions.

### Aliases
CMP and its relatives are just arithmetic without storing the result, with flags enabled. Same as arithmetic with destination x0.

Stack is on memory. If writeback for addresses can be done, the push/pop instructions can be aliased to post increment/decrement memory instructions.

NOP is instruction 0: `add x0 x0 x0`.

# Memory
Two main memory areas: program ROM and RAM. ROM can only be executed, not read by load/store, and RAM cannot be executed.

Sprite ROM and data ROM could also be extra memory areas accessible by load/store.

Separate memory areas for program and data seems simpler than also adding read capability to program ROM for data, as it could collide with regular memory access. In the interest of speed, keep them separate.

A fast RAM is available in the TechnicalFactorio subreddit, a fast compact ROM can be feasibly built with 2-tick read speeds (1 to select cell, 1 to filter signal).

Additionally keep address space small to keep addresses small, within 16-bits or 64KB.

## Memory map
16K R/O space + 16K R/W space, including I/O and display ranges. This gives 32K addresses, which is addressable with 15 bits, or a format 2 immediate16 without going negative.

- Program ROM (separate space): 4K
- Data ROM: 4K (TODO)
- Unused ROM: 8K
- RAM: 8K
- GPIO: 1K
- VRAM: size of framebuffer, probably read-only for load/store and writable through PPU

# Add-ons
## I/O
Buffered memory mapped ports for reading and writing from and to other devices.

32/48/64 long queue, reads/writes to one address, remaining to read/write on another address.

## Interrupts
A special interrupt enable setting and an interrupt signal input.

## PRNG
Give a memory mapped register to seed on write and give random on read. Maybe simplify from Ellipse by just having it cycle constantly instead of only on read, it will still be deterministic.

Xorshift with clock as input, added with a seed register.

## Display
Resolution: around 32x64?
Memory mapped VRAM
Full color if fast enough, black & white for first try.

### PPU
Look at NES, DMG and AGB for inspiration (finally study the AGB graphics stack).

#### Own ideas
Save sprites in special memory regions, issue commands to PPU to draw to the FB.

Steal 1-tick super compact memory from reddit, add dispatchable functions like clear, draw pixel and draw sprite, lock this memory region off while PPU is working.

32x32 screen will take 17 seconds to clear if done one pixel per tick. Similar for double buffering if copying back and forth. Solution: add special function to clear screen to black or some select colors. Add buffer to output which can only be written by one special instruction and is not addressable.

Sprite transparency with -1 color, which is not a valid color anyways. Special function for this.

Decide if will ise a fixed sprite size or variable.

Consider using rgb565 in instructions and convert to rgb888 in transit to PPU, add flag for instructions with colors to use 16-bit rgb from registers

##### Instructions
1. Clear screen, either with some predefined colors or a color in a register.
2. Push FB to display
3. Draw sprite at x,y: flags for transparent (if it cannot be implemented in a fast manner), register address. If space for address too small, make it an offset of VRAM start.
4. Draw pixel at x,y: same color definition as 1.

### Load data
Load data into RAM and VRAM (sprites) the same way Horizon loaded its entire program, or make part of the memory space read-only and paste the program ROM on top of it, which would be simpler.

# TODO
- Pipeline stages
  - [x] fetch
  - [x] decode
  - [x] read registers
  - [x] ex: execute
  - [x] ex: memory access
  - [x] writeback
    - [x] writeback secondary
- Instructions
  - [x] alu
  - [x] conditional execution
  - [x] branching
    - [x] linking
  - [x] load/store
    - [x] post inc/dec
  - [ ] control
    - [ ] set flags
    - [ ] break
    - [ ] wait
- Registers
  - [x] zero
  - [x] general purpose (includes sp, lr)
  - [x] pc
  - [ ] timers
- Add-ons
  - [ ] I/O
    - [ ] Player movement
    - [ ] Text input
  - [ ] Display
    - [ ] PPU
      - [ ] State register
      - [ ] Instruction dispatch
