// 0|1|0|1

import "err"

let a = 0
loop {
	if a >= 2 {
		break
	}

	let b = 0
	loop {
		if b >= 2 {
			break
		}

		err.println(b)
		b = b + 1
	}

	a = a + 1
}
