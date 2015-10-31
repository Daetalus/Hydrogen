
The Hydrogen Programming Language
---------------------------------

Hydrogen is a small, lightweight programming language.
It's intepreted, dynamically typed, and sports a blazing fast tracing JIT compiler.
It also has a C API, which allows you to embed Hydrogen in your own programs.


## Installation

Hydrogen is built using the CMake build system:

```bash
$ git clone https://github.com/GravityScore/Hydrogen
$ cd Hydrogen
$ mkdir build
$ cd build
$ cmake ..
$ make
$ sudo make install
```

This clones the GitHub repository, builds Hydrogen, and installs it on your system.

You can run Hydrogen code using the command line interface:

```bash
$ hydrogen test.hy
```

You can start a REPL session by omitting any options:

```bash
$ hydrogen
```

The command line interface has the following options:

Option        | Description
------------- | -----------
`-h`          | Shows this help information.
`-v`          | Shows the version number of the current Hydrogen installation.
`-b`          | Instead of executing the given program, compiles it and prints its bytecode.
`-s [string]` | Treats the provided string as source code, instead of a path to a file.
`-joff`       | Turns off JIT compilation (slows down program execution).
`-jinfo`      | Shows information about hot loops that are JIT compiled during execution.
