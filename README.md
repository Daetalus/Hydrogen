
Hydrogen
========

The Hydrogen Programming Language.

Hydrogen's a simple programming language. It's object oriented, interpreted, dynamically typed, and garbage collected, with C-style syntax and a (hopefully soon to be) extensive standard library. Here's some sample code:

```rust
fn fib(n) {
	if n == 1 {
		return 1
	} else {
		return fib(n - 1) + fib(n - 2)
	}
}

fib(5)
```

Iterators:

```rust
fn fib(max) {
	n = 0
	current = 1
	previous = 1

	return fn() {
		n += 1
		if n == max {
			return nil
		}

		old_current = current
		current += previous
		previous = old_current
		return current
	}
}

for fib in fib(10) {
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

Hydrogen is intended as more of a learning project for me about writing a bytecode-interpreted language. I'm yet to run any benchmarks to see how it stacks up against the popular languages, but I honestly can't wait.
