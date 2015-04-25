
Hydrogen
========

The Hydrogen Programming Language.

Hydrogen is a simple programming language. It's object oriented, interpreted, dynamically typed, and garbage collected, with C-style syntax and a (hopefully soon to be) extensive standard library.

Hydrogen is intended as more of a learning project for me about writing a bytecode-interpreted language. I've just tested it against Python in the world's most inaccurate and pointless benchmark, and it ran twice as fast! :D


## Building

Make sure you have [CMake](http://www.cmake.org/download/) and GNU Make installed, and build the project like any other CMake project:

```
cd Hydrogen
mkdir build
cd build
cmake ..
make
```

Create your Hydrogen script in a `.hy` file, and execute it with:

```
./hydrogen my-script.hy
```


## Sample Code

```rust
fn fib(n) {
	if n == 1 {
		return 1
	} else {
		return fib(n - 1) + fib(n - 2)
	}
}

io.println(fib(5))
```

Iterators:

```rust
fn fib_iter(max) {
	current = 1
	previous = 1

	return fn() {
		old_current = current
		current += previous
		previous = old_current
		return current
	}
}

for fib in iter.only(fib_iter(10), 10) {
	io.println("The next fibonacci number is", fib)
}
```

Classes:

```rust
// A tree node
class Node {
	fn new(value) {
		self.value = value
		self.children = []
	}

	fn new(value, child) {
		self.value = value
		self.children = [child]
	}

	fn new(value, child1, child2) {
		self.value = value
		self.children = [child1, child2]
	}

	fn print() {
		io.print("[" + self.value)

		for child in self.children {
			io.print(", ")
			child.print()
		}

		io.print("]")
	}
}

tree = new Node(2, new Node(7, new Node(2), new Node(6)), new Node(5))
tree.print() // Will print [2, [7, [2], [6]], [5]]
```
