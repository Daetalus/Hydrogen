import "io"

let a = 5
if a == 5 {
	io.println("a") // expect: a
} else if a == 9 {
	io.println("b")
} else {
	io.println("c")
}


let b = "hello"
if b == "hi" {
	io.println("a")
} else if b == "hello" {
	io.println("b") // expect: b
} else {
	io.println("c")
}

b = nil
if b {
	io.println("a")
} else if b != nil {
	io.println("b")
} else {
	io.println("c") // expect: c
}

io.println("final") // expect: final
