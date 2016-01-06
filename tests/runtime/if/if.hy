// a|b|d|g|h

import "err"

let a = 3
if a == 3 {
	err.println("a")
}

a = "hello"
if a == "hello" {
	err.println("b")
}

if a != "hello" {
	err.println("c")
}

if a != "t" {
	err.println("d")
}

a = false
if a {
	err.println("e")
}

a = nil
if a {
	err.println("f")
}

a = 10.4
if a {
	err.println("g")
}

a = "test"
if a {
	err.println("h")
}
