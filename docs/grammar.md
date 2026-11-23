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
expression-stmt = comparison ";" ;

block          = "{" { statement } "}" ;

comparison     = additive [ ( "==" | "!=" | "<" | ">" ) additive ] ;
additive       = multiplicative [ ( "+" | "-" ) multiplicative ] ;
multiplicative = primary [ ( "*" | "/" ) primary ] ;

primary        = number
               | identifier [ "++" | "--" ]
               | identifier "(" ")"
               | "(" additive ")" ;

identifier     = ( letter | "_" ) { letter | digit | "_" } ;
number         = digit { digit } ;
```

`//` starts a comment that runs to the end of the line.

## Rules the grammar above does not show

- `return;` with no value returns 0.
- The last statement of a function body has to be a `return`.
- A `while` body runs before its condition is first checked.

## Known gaps

- Each level takes one operator at most, so `1 + 2 + 3` and `2 * 3 * 4` do not
  parse. Mixing levels works: `2 * 2 + 3`, `2 * (2 + 3)`.
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
