// 3|4|test|nil|1|2|hello this is a test|another test

import "err"

struct Test {
	a, field, field2, another
}

let a = new Test()

a.a = 3
err.println(a.a)

a.field = 4
err.println(a.field)

a.field2 = "test"
err.println(a.field2)

a.another = nil
err.println(a.another)


struct Test2 {
	a, b, c, aa
}

let b = new Test2()

b.a = 1
err.println(b.a)

b.b = 2
err.println(b.b)

b.c = "hello this is a test"
err.println(b.c)

b.aa = "another test"
err.println(b.aa)
