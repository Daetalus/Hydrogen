// 0|1|0|1

import "err"

let a = 0
while a < 100 {
	let b = 0
	while b < 100 {
		if b >= 2 {
			break
		}
		err.println(b)
		b = b + 1
	}

	if a >= 2 {
		break
	}
	a = a + 1
}
