// 2|1|2 1|5|7|1|2|3|5|8

import "err"

struct Test {
	a, b
}

fn (Test) new() {
	self.a = 2
	self.b = 1
}

let a = new Test()
err.println(a.a)
err.println(a.b)


struct Test2 {
	a, b
}

fn (Test2) new(a, b) {
	self.a = a
	self.b = b
}

let b = new Test2(2, 1)
err.println(b.a, b.b)


struct Test3 {
	a
}

fn (Test3) new(a) {
	if a > 10 {
		self.a = 5
		return
	}

	self.a = a
}

let c = new Test3(11)
err.println(c.a)

let d = new Test3(7)
err.println(d.a)


struct Fib {
	current, previous
}

fn (Fib) new() {
	self.previous = 0
	self.current = 1
}

fn (Fib) next() {
	let temp = self.previous
	self.previous = self.current
	self.current = temp + self.current
	return self.current
}

let fib = new Fib()
err.println(fib.next())
err.println(fib.next())
err.println(fib.next())
err.println(fib.next())
err.println(fib.next())
