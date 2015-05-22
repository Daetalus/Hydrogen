
class Test {
	a, b
}

fn (Test) test() {
	assert(self.a == 3)
}

let a = new Test()
a.a = 3
a.test()
