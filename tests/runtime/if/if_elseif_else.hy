// a|b|c

import "err"

let a = 5
if a == 5 {
	err.println("a")
} else if a == 9 {
	err.println("b")
} else {
	err.println("c")
}


let b = "hello"
if b == "hi" {
	err.println("a")
} else if b == "hello" {
	err.println("b")
} else {
	err.println("c")
}

b = nil
if b {
	err.println("a")
} else if b != nil {
	err.println("b")
} else {
	err.println("c")
}
