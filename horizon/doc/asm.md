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
.var identifier = value
```
where `identifier` is as defined above, and `value` is either a literal as defined
above or a `.const` identifier.

This identifier then acts as a pointer to a single memory address, which contains the
value.

### Array
The `.array` directive is used like so:
```
.array identifier [ value ] = { value-list }
```
where `identifier` is as defined above, `value` is as defined above, and `value-list`
is a comma-separated list of `value`s.

This identifier then acts like a pointer to an array in memory, with `value` items.
The `value-list` may be up to `value` items long, and these are the values the array
is initialized to. If the list is shorter than `value` specifies, the missing values
are assumed to be zero and initialized as such.

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

### @rep...@end
The `@rep` and `@end` preprocessor keywords were scrapped, as there seems to be no
real problem they meaningfully solve.

### Include and define
Directives for the unimplemented include and define concepts may be added later.

## Labels
Labels are as they were previously:
```
identifier:
```
where `identifier` is as defined above.

## Reserved identifiers
### RAM
The `ram` identifier is reserved: it cannot be defined but can be referenced as a
constant. It points to the address after the last instruction in the program, i.e.
freely usable memory.
