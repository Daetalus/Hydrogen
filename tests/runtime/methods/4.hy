
class Test {
	a, b
}

fn (Test) test() {
	self.a = 3
}

let a = new Test()
assert(a.a == nil)
a.test()
assert(a.a == 3)
