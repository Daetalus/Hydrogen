
struct Test {
	a, b
}

let c = 2

fn (Test) test() {
	c = 3
}

let a = new Test()

assert(c == 2)
a.test()
assert(c == 3)
