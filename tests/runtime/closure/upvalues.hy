// 1|2|3|4|1|2|3|5|8|13

import "err"

fn adder() {
	let a = 0
	return fn() {
		a = a + 1
		return a
	}
}

let b = adder()
err.println(b())
err.println(b())
err.println(b())
err.println(b())


fn fib() {
	let previous = 0
	let current = 1
	return fn() {
		let temp = previous
		previous = current
		current = current + temp
		return current
	}
}

b = fib()
err.println(b())
err.println(b())
err.println(b())
err.println(b())
err.println(b())
err.println(b())
