
class Test {
	a, b
}

fn (Test) test(arg) {
	assert(arg == 3)
}

let a = new Test()
a.test(3)
