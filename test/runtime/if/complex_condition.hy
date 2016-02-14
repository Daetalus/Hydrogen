import "io"

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
