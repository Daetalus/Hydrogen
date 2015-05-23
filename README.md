
Hydrogen
========

The Hydrogen Programming Language.

Hydrogen is a simple programming language. It's object oriented, interpreted, dynamically typed, and garbage collected, with C-style syntax and a (hopefully soon to be) extensive standard library.

Hydrogen is intended as more of a learning project for me about writing a bytecode-interpreted language. I've just tested it against Python in the world's most inaccurate and pointless benchmark, and it ran twice as fast! :D


## Building

Make sure you have [CMake](http://www.cmake.org/download/) and GNU Make installed, and build the project like any other CMake project:

Clone the repository and `cd` into the folder:

```
git clone https://github.com/GravityScore/Hydrogen
cd Hydrogen
```

Create a build directory:

```
mkdir build
cd build
```

Generate the build scripts using CMake (defaults to Unix Makefiles):

```
cmake ..
```

Build the project:

```
make
```

This will create a `hydrogen` executable file, which you can use to run your scripts. Create a file with a `.hy` extension and execute it with:

```
./hydrogen my-script.hy
```


## Sample Code

Classic hello world:

```rust
io.println("Hello, World!")
```

Fibonacci:

```rust
fn fib(n) {
	if n == 1 {
		return 1
	}
	return fib(n - 1) + fib(n - 2)
}

io.println(fib(5))
```

Fibonacci with iterators:

```rust
fn fib_iter(max) {
	let current = 1
	let previous = 1

	return fn() {
		let old = current
		current += previous
		previous = old
		return current
	}
}

for fib in iter.take(fib_iter(10), 10) {
	io.println("The next fibonacci number is", fib)
}
```

Classes:

```rust
class Node {
	value,
	children
}

fn (Node) new(value, children) {
	self.value = value
	self.children = children
}

fn (Node) print() {
	io.print("[" + self.value)

	for child in self.children {
		io.print(", ")
		child.print()
	}

	io.print("]")
}

tree = new Node(2, [
	new Node(7, [
		new Node(2, []),
		new Node(6, []),
	]),
	new Node(5, []),
])

tree.print() // Will print [2, [7, [2], [6]], [5]]
```
