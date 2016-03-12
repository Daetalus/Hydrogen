
import "io"

struct Test {
	field,
}

let t = new Test()

t.field = 1
io.println(t.field) // expect: 1

t.field = "hello"
io.println(t.field) // expect: hello


struct Test2 {
	field1, field2
}

let b = new Test2()

b.field1 = 1
b.field2 = 11
io.println(b.field1) // expect: 1
io.println(b.field2) // expect: 11

b.field2 = 20
io.println(b.field1) // expect: 1
io.println(b.field2) // expect: 20


struct Test3 {
	field1, field2, field3
}

let c = new Test3()

c.field1 = "hello"
c.field2 = "nothing"
c.field3 = 3
io.println(c.field1) // expect: hello
io.println(c.field2) // expect: nothing
io.println(c.field3) // expect: 3

c.field1 = 10
c.field3 = 20
io.println(c.field1) // expect: 10
io.println(c.field2) // expect: nothing
io.println(c.field3) // expect: 20
