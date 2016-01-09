// a|b|c

import "err"

let a = 3
if a == 3 {
	err.println("a")
} else if a == 5 {
	err.println("b")
} else if a == 7 {
	err.println("c")
} else {
	err.println("d")
}

a = "test"
if a == 3 {
	err.println("a")
} else if a == "test" {
	err.println("b")
} else if a == "hi" {
	err.println("c")
} else if a == 3.1415 {
	err.println("d")
} else {
	err.println("e")
}

a = 9
if a == 5 {
	err.println("a")
} else if a == false {
	err.println("b")
} else {
	err.println("c")
}
