# Horizon ASM version 2

## Instructions
Syntax is the same as before, the instruction's name followed by its arguments.
The same applies to macros, as they are basically instructions when used.

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

### @rep...@end
The `@rep` and `@end` preprocessor keywords were scrapped, as there seems to be no
real problem they meaningfully solve.

### Include
Directives for the unimplemented include concept may be added later.

## Labels
Labels are as they were previously:
```
identifier:
```
where `identifier` is as defined above.
