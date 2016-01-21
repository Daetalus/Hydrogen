
Hydrogen Source Code
--------------------

Hydrogen is organised into 2 libraries and an executable:

Library         | Source Folder | Description
--------------- | ------------- | -----------
`libhydrogen.a` | `vm/`         | Hydrogen's core - the bytecode parser, interpreter, and JIT compiler
`libhystdlib.a` | `std/`        | The standard library, implemented using public functions exported by the core
`cli`           | `cli/`        | The command line interface for executing files and running a REPL

All include files are in `/include`. The `hydrogen.h` file is for the core interpreter, and `hystdlib.h` can be included to add the standard library to an instance of the interpreter.

Further documentation for each of these components is located in their respective folders.
