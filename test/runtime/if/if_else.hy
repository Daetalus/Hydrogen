import "io"

let c = 10
if c > 5 {
	io.println("a") // expect: a
} else {
	io.println("b")
}

c = 9
if c < 4 {
	io.println("a")
} else {
	io.println("b") // expect: b
}

let thing = false
if thing {
	io.println("a")
} else {
	io.println("b") // expect: b
}

io.println("final") // expect: final
