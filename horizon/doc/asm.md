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
