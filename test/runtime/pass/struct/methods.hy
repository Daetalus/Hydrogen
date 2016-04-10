
import "io"

struct Test {
	field
}

fn (Test) test() {
	return 3
}

let t = new Test()
io.println(t.test()) // expect: 3


fn (Test) test2(arg) {
	return arg + 1
}

io.println(t.test2(4)) // expect: 5


fn (Test) test3(arg1, arg2) {
	return arg1 + arg2
}

io.println(t.test3(2, 5)) // expect: 7


fn (Test) test4() {
	return t.field + 1
}

t.field = 3
io.println(t.test4()) // expect: 4

t.field = 5
io.println(t.test4()) // expect: 6


fn (Test) test5(arg) {
	t.field = arg + 1
}

t.test5(3)
io.println(t.field) // expect: 4
t.test5(10)
io.println(t.field) // expect: 11


fn (Test) something() {
	return t.field
}

t.field = 1
let a = t.something
io.println(a()) // expect: 1

t.field = 12
io.println(a()) // expect: 12
