
Hydrogen Programming Language
-----------------------------

Hydrogen is a small, lightweight programming language with C-style syntax. It's intepreted, dynamically typed, and sports a blazingly fast tracing JIT compiler, along with an easy to use C API, allowing you to quickly embed Hydrogen in your own programs.

It was written as more of a educational exercise for me, rather than anything intended for production use.

Hydrogen is written in C and has no external dependencies. The tests use the [Google C++ Testing Framework](https://github.com/google/googletest).


### Installation

Hydrogen needs to be build from its source code, which can be done using the [CMake](https://cmake.org/) build system:

```bash
$ git clone --recursive https://github.com/GravityScore/Hydrogen
$ cd Hydrogen
$ mkdir build
$ cd build
$ cmake ..
$ make
```

-This clones the GitHub repository and builds Hydrogen and its tests.

You can run Hydrogen code using the command line interface:

```bash
$ hydrogen test.hy
```

You can start a REPL session (well, once I implement it) like so:

```bash
$ hydrogen
```

The command line interface has a number of options:

Option    | Description
--------- | -----------
`-h`      | Shows this help information.
`-v`      | Shows the version number of the current Hydrogen installation.
`-b`      | Prints the program's compiled bytecode.
`-s`      | Doesn't execute a file, but rather treats the command line arguments as Hydrogen source code.
`--joff`  | Turns off JIT compilation during runtime.
`--jinfo` | Shows information about hot loops that are JIT-compiled during execution.


### License

Hydrogen is licensed under the MIT license. This basically means you can do whatever you want with the code. See the `LICENSE` file for more details.
