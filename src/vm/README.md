
Hydrogen Core
-------------

When you call `hy_run` or `hy_run_file`, Hydrogen follows a number of steps:

1. Lex the source code provided
2. Parse the source code into bytecode
3. Interpret the bytecode
4. Detect hot loops
5. Record a trace through a hot loop
6. Compile the trace into Single Static Assignment for Intermediate Representation (SSA IR)
7. Optimise the IR
8. Compile the IR into machine code
9. Execute the machine code until we return to the bytecode interpreter

Let's run through these individually.


## Lexing and Parsing

* Main entry point: `hy_run` in `vm.c`
* Parser entry point: `parse_pacakge` in `parser/parser.c`
* Next token from lexer: `lexer_next` in `parser/lexer.c`
* List of tokens outputted by the lexer: `TokenType` in `parser/lexer.h`

The main entry point for executing a source file is the `hy_run` function in `vm.c`. Code is compiled in units of *packages*. Each package has an associated source code string (whether that be from a file or given to the interpreter in a string). The *main package* is the first package created using the file or source string given to the `hy_run` function. Extra packages are created and compiled as `import` statements are parsed in existing packages. A package is compiled into bytecode by the *parser*. This is done by calling the function `parse_package` in `parser/parser.c`.

Lexing and parsing a package is done in a single pass over the source code. This makes Hydrogen's source code simpler, and reduces the time needed to compile a package. A *lexer* is created from the package's source code. Each time `lexer_next` in `parser/lexer.c` is called, the lexer emits the next token it finds in the source code, until it hits the end of the file. These tokens are parsed into recognisable structures.


## Interpreter

* Main entry point: `fn_exec` in `vm.c`
