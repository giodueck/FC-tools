# Implementation details

## Opcodes

(s) indicates that the instruction can be suffixed with 's' to update the flags
(l) indicates that the instruction can be suffixed with 'l' to write a return address to RA

0: halt: halt clock

8: mov(s): mov reg
9: movi(s): mov s16 imm
10: movl(s): mov u16 imm, or with MS half-word currently in register
11: movh(s): mov s16 imm lsh 16, or with LS half-word currently in register

16: add(s): add reg with reg, and so on
17: sub(s)
18: mul(s)
19: div(s)
20: mod(s)
21: pow(s)
22: lsh(s)
23: rsh(s)
24: and(s)
25: or(s)
26: xor(s)

32: addi(s): add reg with s12 imm, and so on
33: subi(s)
34: muli(s)
35: divi(s)
36: modi(s)
37: powi(s)
38: lshi(s)
39: rshi(s)
40: andi(s)
41: ori(s)
42: xori(s)

48: jeq(l): jump to u16 imm if flags (Z)
49: jne(l): jump to u16 imm if flags (!Z)
50: jlt(l): jump to u16 imm if flags (N != V)
51: jgt(l): jump to u16 imm if flags (!Z & N = V)
52: jle(l): jump to u16 imm if flags (Z & N != V)
53: jge(l): jump to u16 imm if flags (N = V)
54: jng(l): jump to u16 imm if flags (N)
55: jpz(l): jump to u16 imm if flags (!N)
56: jvs(l): jump to u16 imm if flags (V)
57: jvc(l): jump to u16 imm if flags (!V)
58: jmp(l): jump to u16 imm

64: jeqr(l): jump to reg if flags (Z)
65: jner(l): jump to reg if flags (!Z)
66: jltr(l): jump to reg if flags (N != V)
67: jgtr(l): jump to reg if flags (!Z & N = V)
68: jler(l): jump to reg if flags (Z & N != V)
69: jger(l): jump to reg if flags (N = V)
70: jngr(l): jump to reg if flags (N)
71: jpzr(l): jump to reg if flags (!N)
72: jvsr(l): jump to reg if flags (V)
73: jvcr(l): jump to reg if flags (!V)
74: jmpr(l): jump to reg

80: storer: store reg to reg address
81: storei: store red to u16 imm address
82: storeri: store reg to reg address and increment address
83: storerd: store reg to reg address and decrement address

84: loadr(s): load reg to reg address
85: loadi(s): load red to u16 imm address
86: loadri(s): load reg to reg address and increment address
87: loadrd(s): load reg to reg address and decrement address

88: push: push reg to stack
89: pushi: push s16 imm to stack
90: pop(s): pop from stack into reg

96: ramcp: copy u16 imm number of words into RAM block starting at reg address
97: poprx: pop the top element from the receive stack
98: poptx: pop the top element from the transmit input stack
99: flushtx: flush the transmit input stack to the output

## Control signals
M = instruction word

(M >> 24) &   255 = O opcode
(M >> 23) &     1 = F flags/link
(M >> 18) &    31 = D dest reg
(M >> 12) &    31 = T op1 reg
(M >> 0)  &    31 = S op2 reg
(M >> 0)  & 65535 = U u16
(M >> 0)  &  4095 = V u12
(M << 16) >>   16 = I s16
(M << 20) >>   20 = J s12
(M >> 17) &     1 = G switch operands

H = load upper HW from bus A to result
L = load lower HW from bus A to result
E = load immediate as A instead of B

### Signals per opcode
Results:
- R = result register
- A = register to load at A
- B = register to load at B
- I/J/U/V = immediate to load as A/B
- F = activate flags
- L = activate linking
- O = ALU operation
- C = conditional operation
- X = pass A to R
- Y = pass B to R
- Z = pass A & -65536 to R
- W = pass B & 65535 to R

MOV: D->R, F->F
8: S->A, X
9: I->I, X
10: U->U, Z
11: I->I, W
