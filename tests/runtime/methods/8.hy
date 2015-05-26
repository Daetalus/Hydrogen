
struct Test {
	a, b
}

fn (Test) new(a) {
	self.a = a
	if a == 3 {
		self.b = 8
		return
	}
}

let i = new Test(10)
assert(i.a == 10)
assert(i.b == nil)

let j = new Test(3)
assert(j.a == 3)
assert(j.b == 8)
