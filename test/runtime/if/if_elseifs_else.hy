import "io"

let a = 3
if a == 3 {
	io.println("a") // expect: a
} else if a == 5 {
	io.println("b")
} else if a == 7 {
	io.println("c")
} else {
	io.println("d")
}

a = "test"
if a == 3 {
	io.println("a")
} else if a == "test" {
	io.println("b") // expect: b
} else if a == "hi" {
	io.println("c")
} else if a == 3.1415 {
	io.println("d")
} else {
	io.println("e")
}

a = 9
if a == 5 {
	io.println("a")
} else if a == false {
	io.println("b")
} else {
	io.println("c") // expect: c
}
