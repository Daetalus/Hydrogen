//

struct Fib {
	current, previous
}

fn (Fib) next() {
	let temp = self.previous
	self.previous = self.current
	self.current = self.previous + self.current
	return self.current
}

let fib = new Fib()
fib.current = 0
