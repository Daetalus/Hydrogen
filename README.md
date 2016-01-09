
Hydrogen Programming Language
-----------------------------

Hydrogen is a small, lightweight programming language with C-style syntax. It's intepreted, dynamically typed, and sports a blazingly fast tracing JIT compiler, along with an easy to use C API, allowing you to quickly embed Hydrogen in your own programs.

It was written as more of an educational exercise for me, rather than anything intended for production use.

Hydrogen is written in C and has no external dependencies. The tests use the [Google C++ Testing Framework](https://github.com/google/googletest).


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

And the infamous Hello World:

```
import "io"
io.println("Hello, world!")
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

You can start a REPL session (well, once I implement it) like so:

```
$ hydrogen
```

The command line interface has a number of options:

Option            | Description
----------------- | -----------
`-b`              | Prints a program's compiled bytecode.
`--stdin`         | Read program source code from the standard input.
`--joff`          | Turns off JIT compilation during runtime.
`--jinfo`         | Shows information about hot loops that are JIT-compiled during execution.
`--help`, `-h`    | Shows this help information.
`--version`, `-v` | Shows the version number of the current Hydrogen installation.


## License

Hydrogen is licensed under the MIT license. This basically means you can do whatever you want with the code. See the `LICENSE` file for more details.
