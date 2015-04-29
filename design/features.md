
Features
========


# Language

```
// comment

/* block comment */

let a = 3
a = 5

if a == 5 {

} else if a == 3 {

} else {

}

loop {
	io.println("infinite!")
	if a == 5 {
		break
	}
}

let i = 0
while i < 10 {
	io.println("hello")
	i += 1
}

for i in iter.range(10) {
	io.println(i)
}

fn test(arg) {
	io.println(arg)
}

fn test(arg, arg2) {
	io.println("second version", arg, arg2)
}

test("hello")
test("hai", "test")

fn test(items...) {
	for item in items {
		io.println(item)
	}
}

test("thing 1", 2, 3, "thing 4")

let global = 3

fn closure() {
	global = 4
}

io.println(global)
closure()
io.println(global)

let array = [1, 2, 3, 4]
for item in array.iter() {
	io.println(item)
}

io.println(array[0])

let dict = {"hello": 123, "hai": "testing", "4": "meh"}
for key in dict.iter() {
	io.println(key)
}

io.println(dict["hello"])

(fn(a, b) {
	io.println(a, b)
})(1, 2)

class Thing {
	name,
	birth,
}

fn (Thing) new(name) {
	self.name = name
}

fn (Thing) print() {
	io.println(self.name)
}

fn (Thing) print_birth() {
	io.println(self.birth)
}

let a = new Thing("Nothing")
a.print()
a.print_birth()









fn test(test) {
	println(test)
}

let test = 3
test(3)

-------

class Thingy {
	name,
	date,
}

fn (Thingy) test() {
	println("hai")
}

let test = new Thingy()
test.test()

----

(fn(a, b) {
	println(a, b)
})(1, 2)

```

[] Comments
[x] Variable assignments
[x] Mathematical assignments
[x] Strings/numbers/booleans/nil
[x] If statements
[x] While loops
[x] Break
[] For loops
[x] Iterators
[x] Function calls
[x] Function definitions
[x] Return values
[x] Closures
[] Classes
[] Constructors
[] Properties
[] Methods
[] Superclasses
[] Modules
[] Module variables
[] Arrays
[] Dictionaries
[] Variable function arguments
[] Standard library
[] Calling results of expressions
[] Multiple levels of indirection with . operator

Function calls cannot have newlines between opening parenthesis and the identifier, eg:
test
(a, b, c)

is illegal
