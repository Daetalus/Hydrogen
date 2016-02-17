
Hydrogen Source Code
--------------------

Hydrogen is organised into 2 libraries and an executable, each of which has their own subfolder:

Library         | Source Folder | Description
--------------- | ------------- | -----------
`libhydrogen.a` | `core/`       | The Hydrogen core - bytecode parser, interpreter, and JIT compiler
`libhylib.a`    | `lib/`        | The standard library, implemented using functions exported by the core
`cli`           | `cli/`        | The command line interface for executing files and running the REPL

All include files are in `/include`. The `hydrogen.h` file is for the core, and `hylib.h` can be included to add the standard library to an instance of the interpreter.
