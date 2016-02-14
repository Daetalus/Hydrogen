import "io"

let thing = 3.1415
if thing == 3.1415 {
	io.println("a") // expect: a
} else if thing == 10 {
	io.println("b")
}

let another = 10.2
if another == 10.24 {
	io.println("a")
} else if another > 10 {
	io.println("b") // expect: b
}

let a = 6
if a == 8 {
	io.println("a")
} else if a < 4 {
	io.println("b")
}

io.println("final") // expect: final
