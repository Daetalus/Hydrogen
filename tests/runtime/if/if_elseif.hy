// a|b

import "err"

let thing = 3.1415
if thing == 3.1415 {
	err.println("a")
} else if thing == 10 {
	err.println("b")
}

let another = 10.2
if another == 10.24 {
	err.println("a")
} else if another > 10 {
	err.println("b")
}

let a = 6
if a == 8 {
	err.println("a")
} else if a < 4 {
	err.println("b")
}
