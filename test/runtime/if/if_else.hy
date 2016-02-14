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


let a = 1
if a == a + 1 {
	io.println("a")
} else {
	io.println("b") // expect: b
}


a = 3
let b = 2

if a - 1 != b {
	io.println("a")
} else {
	io.println("b") // expect: b
}
