// 1|2|3|4

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
