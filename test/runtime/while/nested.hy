// 0|1|0|1

import "err"

let a = 0
while a < 2 {
	let b = 0
	while b < 2 {
		err.println(b)
		b = b + 1
	}
	a = a + 1
}
