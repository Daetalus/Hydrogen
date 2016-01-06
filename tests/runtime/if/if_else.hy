// a|b|b

import "err"

let c = 10
if c > 5 {
	err.println("a")
} else {
	err.println("b")
}

c = 9
if c < 4 {
	err.println("a")
} else {
	err.println("b")
}

let thing = false
if thing {
	err.println("a")
} else {
	err.println("b")
}
