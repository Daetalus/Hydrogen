
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
for item in array {
	io.println(item)
}

io.println(array[0])

let dict = {"hello": 123, "hai": "testing", 4: "meh"}
for key in dict {
	io.println(key)
}

io.println(dict["hello"])

class Thing {
	fn new() {
		self.thing = 1
	}

	fn print() {
		io.println(self.thing)
	}

	fn thing() {
		return self.thing
	}
}

let a = new Thing()
io.println(a.test() + 3)
```

* Comments
* Variable assignments
	* Mathematical assignments
* Strings/numbers/booleans/nil
* If statements
* While loops
* For loops
* Iterators
* Function calls
* Function definitions
* Return values
* Closures
* Classes
	* Constructors
	* Properties
	* Methods
* Arrays
* Dictionaries
* Variable function arguments
* Standard library
