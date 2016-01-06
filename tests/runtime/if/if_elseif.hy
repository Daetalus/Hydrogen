// a|b

import "err"

let thing = 3.1415
if thing - 3.1415 < 0.0001 {
	err.println("a")
} else if thing == 3.1415 {
	err.println("b")
}

let another = 10.2
if another == 10.24 {
	err.println("a")
} else if another > 10 {
	err.println("b")
}
