# Horizon Grammar
```
statement   : instruction
            | directive
            | label
            ;

directive   : .const identifier literal
            | .var identifier value
            | .array identifier [ value ] { value-list }
            | .start
            | .name comment
            | .desc comment-series
            | .macro identifier (literal)? newline macrodef
            ;

macrodef    : macroline newline macrodef
            | macroline
            ;

macroline   : . instruction // with the macro parameters used like shell script
            ;               // arguments: $1$, $2$ ...

(pseudo grammar definitions)
comment     : // ';' until the end of the line
comment-series : comment \n comment-series
               | not-comment
               ;

label       : identifier : // must be undefined
            ;

value       : literal
            | identifier // must be a const
            ;

value-list  : value , value-list
            | value
            ;

instruction : parse_noop
            | parse_alu
            | parse_not
            | parse_ram
            | parse_cond
            | parse_push
            | parse_pop
            ;

parse_noop  : "noop"
            ;

parse_alu   : alu_normal format-2
            | alu_normal format-3
            ;

parse_not   : "not" format-6
            ;

parse_pop   : "pop" format-4
            ;

parse_ram   : ram_instr format-4
            | ram_instr format-5
            ;

parse_cond  : cond_instr format-4
            | cond_instr format-5
            ;

parse_push  : "push" format-4
            | "push" format-5
            ;

format-4    : register
            ;

format-5    : imm16
            ;

format-6    : register register
            : register
            ;

format-2    : register register register
            | register register
            ;

format-3    : register register imm8
            | register imm8
            ;
```
