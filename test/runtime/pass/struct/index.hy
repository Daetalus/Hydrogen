
import "io"

struct Test {
	field1, field2
}

let t = new Test()
t.field1 = 10
t.field2 = 20

io.println(t.field1 + t.field2) // expect: 30
io.println(t.field1 + 100) // expect: 110

let c = t.field2
io.println(c) // expect: 20


fn something(a_struct) {
	io.println(a_struct.field1)
}

something(t) // expect: 10


let t2 = new Test()
t2.field1 = 20
t2.field2 = 40

io.println(t.field1 + t2.field1) // expect: 30
io.println(t2.field2 + 60) // expect: 100


t2.field1 = t
io.println(t2.field1.field1) // expect: 10


// Cyclic references
t.field1 = t2
io.println(t.field1.field1.field2) // expect: 20
io.println(t.field1.field1.field1.field2) // expect: 40
