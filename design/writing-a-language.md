
Writing a Language
==================


We're given a source code file (a string), and we need to execute it. There are a few steps involved:

* Lexing
* Compilation
* Execution


## Definitions

A few definitions first:

* **Identifier** - A sequence of letters (lower or upper case), numbers, or underscores. Must not start with a number.
* **Bytecode** - Like assembly code. Simple instructions that we can easily execute.


## Lexing

Lexing takes in the source code string, and outputs a stream of tokens. A token is the smallest unit of recognisable syntax. The tokens emitted by the lexer are:

Mathematical operators:

* Addition (`+`)
* Subtraction (`+`)
* Multiplication (`+`)
* Division (`+`)
* Modulo (`+`)

Boolean operators:

* Boolean and (`&&`)
* Boolean or (`||`)
* Boolean not (`!`)
* Equal (`==`)
* Not equal (`!=`)
* Less than (`<`)
* Less than or equal to (`<=`)
* Greater than (`>`)
* Greater than or equal to (`>=`)

Bitwise operators:

* Left shift (`<<`)
* Right shift (`>>`)
* Bitwise and (`&`)
* Bitwise or (`|`)
* Bitwise not (`~`)
* Bitwise xor (`^`)

Assignment:

* Assign (`=`)
* Addition assignment (`+=`)
* Subtraction assignment (`-=`)
* Multiplication assignment (`*=`)
* Division assignment (`/=`)
* Modulo assignment (`%=`)

Syntax:

* Open parenthesis (`(`)
* Close parenthesis (`)`)
* Open bracket (`[`)
* Close bracket (`]`)
* Open brace (`{`)
* Close brace (`}`)
* Dot (`.`)
* Comma (`,`)

Keywords:

* Let (`let`)
* If (`if`)
* Else (`else`)
* Else If (`else if`)
* While (`while`)
* Loop (`loop`)
* Break (`break`)
* For (`for`)
* In (`in`)
* Function (`fn`)
* True (`true`)
* False (`false`)
* Nil (`nil`)

Other:

* Identifiers (eg. `test` or `_variable_1`)
* Numbers (eg. `13` or `42.8`)
* String literals (eg. `"test"` or `'hello'`)
* Newlines (`\n` or `\r`)
* End of file

The lexer gives the source code to a parser. The parser helps the lexer sequentially extract bits of the source code. To do this, it has a cursor - a position within the source code string. The cursor position is incremented to iterate over characters. This allows easy extraction of numbers, identifiers, etc. It also provides simple functions like `current_char`, which returns the character under the cursor, `starts_with`, to test if the string at the current cursor position equals something, and `is_eof`, which returns true if the cursor has reached the end of the file.

For example, say we have the source code `let a = 3`. The parser starts with a cursor position of 0 (the first character in the source). We can look at the character at the current cursor position, and sees that it's a letter (the letter `l`). We could call the function `consume` to increment the current cursor position, or call `consume_identifier` to consume a whole identifier, etc. Take a look at `parser.h` to see all the available functions.

The lexer uses the parser and its functionality to extract tokens from the source. To parse a token, it first consumes all spaces and tabs. If the next character is a letter, then we have an identifier, so we consume it and emit an identifier token. If the next character is a number, then we use the parser to extract the number, and emit a number token. If it's a quote (`"` or `'`), then we have a string literal. If the character is `(`, for example, then we emit an open parenthesis token, etc.


## Compilation

The compiler uses a lexer, and interprets a sequence of tokens as a statement, which it generates bytecode for. There are 3 major constructs used to structure code:

* **Block** - Contain a sequence of statements, one after each other. Usually surrounded by `{` and `}`.
* **Statement** - A single language construct that can be treated independently, like a while loop or if/else block. This includes the block attached to any statement. Contains expressions.
* **Expression** - A sequence of operators and operands that will result in a value. For example, the expression `3 + 4 * 5` results in the value of 23.

We begin by treating the contents of the whole file as one big block, terminated by an end of file token.

To compile a block, we just keep compiling statements until we find the block's terminator token, such as a `}`.

To compile a statement, we determine what sort of statement it is (eg. is it a for loop, if statement, etc.), then handle each case separately.

To compile an expression, we use a Pratt parser, explained well by Bob Nystrom in his post [here](http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/). I won't bother explaining it - you can just read the article.

For example, compiling a while loop involves consuming the while keyword, then compiling an expression terminated by an open brace, then consuming the open brace, compiling a block terminated by a close brace, and then consuming the close brace.

### Variable Assignment

### If Statements

### While Loops

### Break Statements

### Function Definitions

### Return Statements

* Functions return by pushing their return value (or nil if the function doesn't have a return value) onto the stack
* When the virtual machine encounters a return instruction, it pops the

### Closures

## Execution
