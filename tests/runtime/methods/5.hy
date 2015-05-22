
class Test {
	a, b
}

fn (Test) test() {
	self.a = 3
}

fn (Test) test2() {
	self.b = 4
}

let a = new Test()
assert(a.a == nil)
assert(a.b == nil)

a.test()
assert(a.a == 3)
assert(a.b == nil)

a.test2()
assert(a.a == 3)
assert(a.b == 4)
