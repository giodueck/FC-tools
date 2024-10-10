# Implementation details

## Opcodes

(s) indicates that the instruction can be suffixed with 's' to update the flags

0: noop: skip instruction, waste one cycle
1: halt: halt clock

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

48: jeq: jump to u16 imm if flags (Z)
49: jne: jump to u16 imm if flags (!Z)
50: jlt: jump to u16 imm if flags (N != V)
51: jgt: jump to u16 imm if flags (!Z & N = V)
52: jle: jump to u16 imm if flags (Z & N != V)
53: jge: jump to u16 imm if flags (N = V)
54: jng: jump to u16 imm if flags (N)
55: jpz: jump to u16 imm if flags (!N)
56: jvs: jump to u16 imm if flags (V)
57: jvc: jump to u16 imm if flags (!V)
58: jmp: jump to u16 imm
59: call: jump to u16 imm and store PC+1 in RA

64: jeqr: jump to reg if flags (Z)
65: jner: jump to reg if flags (!Z)
66: jltr: jump to reg if flags (N != V)
67: jgtr: jump to reg if flags (!Z & N = V)
68: jler: jump to reg if flags (Z & N != V)
69: jger: jump to reg if flags (N = V)
70: jngr: jump to reg if flags (N)
71: jpzr: jump to reg if flags (!N)
72: jvsr: jump to reg if flags (V)
73: jvcr: jump to reg if flags (!V)
74: jmpr: jump to reg
75: callr: jump to reg and store PC+1 in RA

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

96: ramcp: copy u16 imm number of words into reg address, incrementing reg and PC
