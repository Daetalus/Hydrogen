
The Hydrogen Programming Language
---------------------------------

Hydrogen is a small, lightweight programming language with C-style syntax. It's intepreted, dynamically typed, and sports a blazingly fast tracing JIT compiler, along with an easy to use C API, allowing you to quickly embed Hydrogen in your own programs.

 The core language and standard library are written in C and have no external dependencies. The tests are written using a small custom testing framework. The runtime tests are executed using a custom Python script.


## Example

Here's some Hydrogen code:

```rust
import "io"

struct Node {
	name, child
}

fn (Node) new(name, child) {
	self.name = name
	self.child = nil
}

fn (Node) print() {
	io.print("[" .. self.name)
	if self.child {
		io.print(", ")
		self.child.print()
	}
	io.print("]")
}

let root = new Node("1", new Node("2", new Node("3", nil)))
root.print() // Prints [1, [2, [3]]]
```


## Installation

Hydrogen needs to be build from its source code, which can be done using the [CMake](https://cmake.org/) build system:

```
$ git clone --recursive https://github.com/GravityScore/Hydrogen
$ cd Hydrogen
$ mkdir build
$ cd build
$ cmake ..
$ make
```

This clones the GitHub repository and builds Hydrogen and its tests.

You can run Hydrogen code using the command line interface:

```
$ hydrogen test.hy
```

You can start a REPL session like so:

```
$ hydrogen
```

You can run the parser and runtime tests like so:

```
$ ctest
```

The command line interface has a number of options:

Option            | Description
----------------- | -----------
`-b`              | Prints a program's compiled bytecode
`--stdin`         | Read program source code from the standard input
`--joff`          | Turns off JIT compilation during runtime
`--jinfo`         | Shows information about hot loops that are JIT-compiled during execution
`--help`, `-h`    | Shows this help information
`--version`, `-v` | Shows the version number of the current Hydrogen installation


## License

Hydrogen is licensed under the MIT license. This basically means you can do whatever you want with the code. See the `LICENSE` file for more details.


## Structure

The project is organised into a number of folders and files:

Path             | Description
---------------- | -----------
`src/`           | The source code for the core language, standard library, and command line interface
`test/`          | All parser, JIT compiler, and runtime tests
`benchmark/`     | Identical programs written in Hydrogen and a few other languages to compare speed
`include/`       | C header files that need to be included to use the C API
`CMakeLists.txt` | The CMake build script
`LICENSE`        | The MIT license file
