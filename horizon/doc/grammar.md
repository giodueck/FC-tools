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
            ;

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

// TODO
instruction ;
```
