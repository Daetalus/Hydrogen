
struct Test {
	a
}

fn (Test) new(a) {
	self.a = a
}

let i = new Test(10)
assert(i.a == 10)
