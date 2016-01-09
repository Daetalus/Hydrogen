// 3|4|5

import "err"

let a = 3
loop {
	if a > 5 {
		break
	}

	err.println(a)
	a = a + 1
}
