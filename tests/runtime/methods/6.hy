
class Test {
	a
}

fn (Test) new() {
	self.a = 3
}

let i = new Test()
assert(i.a == 3)
