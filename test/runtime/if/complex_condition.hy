// b|b

import "err"

let a = 1
if a == a + 1 {
	err.println("a")
} else {
	err.println("b")
}


a = 3
let b = 2

if a - 1 != b {
	err.println("a")
} else {
	err.println("b")
}