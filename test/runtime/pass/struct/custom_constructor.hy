
import "io"

struct Test {
	field1, field2
}

fn (Test) new() {
	self.field1 = 1
	self.field2 = "apples"
}

let t = new Test()
io.println(t.field1) // expect: 1
io.println(t.field2) // expect: apples

t.field1 = 10
io.println(t.field1) // expect: 10


struct Test2 {
	field
}

fn (Test) new() {
	self.field = "wow"
	io.println(self.field)
	self.hello()
	io.println(self.field)
}

fn (Test) hello() {
	self.field = "hello"
}

// expect: wow
// expect: hello
let b = new Test2()

io.println(b.field) // expect: hello

b.field = "nothing"
io.println(b.field) // expect: nothing


struct Test3 {
	field
}

fn (Test) new(arg1, arg2) {
	self.field = arg1 + arg2
}

let c = new Test3(3, 4)
io.println(c.field) // expect: 7
