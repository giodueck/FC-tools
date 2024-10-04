# Instruction Set Architecture

## Register operations

- mov
- test (mov into x0 with flags enabled)

## Arithmetic and Logic
### First class operations
There are several operations supported directly by arithmetic combinators:

- add
- sub
- cmp (sub into x0 with flags enabled)
- mul
- div
- mod
- pow
- lsh
- rsh
- and
- or
- xor
- not (xor -1)

### Implemented operations
These are operations implemented using circuits of combinators.

- sqrt? (doable in 4 ticks, complex circuitry) [reddit](https://www.reddit.com/r/technicalfactorio/comments/bzjme6/super_fast_scalar_integer_base_2_logarithm_and/)

## Jumping

### Branching
Simple jumping instructions.

- jeq (Z)
- jne (!Z)
- jlt (N != V)
- jgt (!Z & N = V)
- jle (Z & N != V)
- jge (N = V)
- jng (N)
- jpz (!N)
- jvs (V)
- jvc (!V)
- jmp (always)

### Function calls
Branch and link for control to return to the branching point, for subroutine calls. Return can be done by jmp RA.

- call (stores PC+1 in RA)

## Memory access
### Store
Save a value into a memory address.

- store
- storei (store and increment address)
- stored (store and decrement address)

### Load
Load a value from a memory address.

- load
- loadi (load and increment address)
- loadd (load and decrement address)

## Stack access

- push
- pop

## Misc
### ROM to RAM copy
Copy arrays of values from program ROM to RAM.

- ramcp

### PRNG

- noop (instruction 0)
- prng (set register to pseudo-random value)
- seed (seed PRNG)
