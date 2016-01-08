// 0|1|2|3|4|after|yes|yes|yes|yes

import "err"

let n = 0
while n < 100 {
	if n >= 5 {
		break
	}
	err.println(n)
	n = n + 1
}
err.println("after")


n = 1
let a = 3
while a == 3 {
	if n % 5 == 0 {
		break
	}
	n = n + 1
	err.println("yes")
}
