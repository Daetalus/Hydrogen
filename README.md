
Hydrogen Programming Language
-----------------------------

Hydrogen is a small, lightweight programming language with C-style syntax.
It's intepreted, dynamically typed, and sports a blazingly fast tracing JIT compiler,
along with an easy to use C API, allowing you to quickly embed Hydrogen in your own programs.


### Installation

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

This clones the GitHub repository, builds Hydrogen, and installs it on your system (in `/usr/local`).

You can run Hydrogen code using the command line interface:

```bash
$ hydrogen test.hy
```

You can start a REPL session like so:

```bash
$ hydrogen
```

The command line interface has a number of options:

Option   | Description
-------- | -----------
`-h`     | Shows this help information.
`-v`     | Shows the version number of the current Hydrogen installation.
`-b`     | Prints the program's compiled bytecode.
`-s`     | Doesn't execute a file, but rather treats the command line arguments as Hydrogen source code.
`-joff`  | Turns off JIT compilation during runtime (slows down program execution).
`-jinfo` | Shows information about hot loops that are JIT-compiled during execution.
