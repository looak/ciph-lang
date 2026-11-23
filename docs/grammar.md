# Grammar

What the parser accepts today, written in EBNF: `[ x ]` means x is optional,
`{ x }` means x zero or more times, `|` separates alternatives, and text in
quotes is written as is.

```
program        = { statement } ;

statement      = let | return | while | function | expression-stmt ;

let            = "let" identifier "=" comparison ";" ;
return         = "return" [ comparison ] ";" ;
while          = "while" "(" comparison ")" block ;
function       = "fn" identifier "(" ")" block ;
expression-stmt = ( call | inc-dec ) ";" ;

block          = "{" { statement } "}" ;

comparison     = additive [ ( "==" | "!=" | "<" | ">" ) additive ] ;
additive       = multiplicative { ( "+" | "-" ) multiplicative } ;
multiplicative = primary { ( "*" | "/" ) primary } ;

primary        = number
               | identifier
               | inc-dec
               | call
               | "(" additive ")" ;

inc-dec        = identifier ( "++" | "--" ) ;
call           = identifier "(" ")" ;

identifier     = ( letter | "_" ) { letter | digit | "_" } ;
number         = digit { digit } ;
```

`//` starts a comment that runs to the end of the line.

## Rules the grammar above does not show

- `return;` with no value returns 0.
- The last statement of a function body has to be a `return`.
- A `while` body runs before its condition is first checked.
- A chain of `+ -` or of `* /` groups from the left: `10 - 4 - 3` is
  `(10 - 4) - 3`. `*` and `/` bind tighter than `+` and `-`.
- A comparison does not chain: `a == b == c` is an error.
- An expression can stand alone as a statement only when it does something: a
  call, or `++` / `--`. Anything else, such as `1 + 2;` or `x == y;`, is
  reported as `e0xe060`, because its result is not used.

## Known gaps

- A parenthesised expression cannot hold a comparison.
- Functions take no parameters and calls no arguments.
- `if`, `else`, `mut`, `<=`, `>=` and assignment to an existing variable are
  recognised by the lexer but not by the parser.

## What the later stages add

The semantic analyzer rejects programs that parse but make no sense: a name
used before it is declared, or declared twice. Its rules are described in
`semantic_analyzer.hpp`.

The code generator cannot compile everything the parser and the analyzer
accept. Function definitions, and a variable name reused in two sibling
blocks, are reported as `e0xe301: not supported yet`.
