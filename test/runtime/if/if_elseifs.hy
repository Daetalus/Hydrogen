import "io"

let a = 3
if a == 3 {
	io.println("a") // expect: a
} else if a == 4 {
	io.println("b")
} else if a == 5 {
	io.println("c")
}

a = 10
if a == 3 {
	io.println("a")
} else if a == 10 {
	io.println("b") // expect: b
} else if a == 9 {
	io.println("c")
}

a = 11
if a == 10 {
	io.println("a")
} else if a == 3 {
	io.println("b")
} else if a == 11 {
	io.println("c") // expect: c
} else if a == 5 {
	io.println("d")
}

a = 70000
if a == 12.4 {
	io.println("a")
} else if a > 80000 {
	io.println("b")
} else if a < 2 {
	io.println("c")
} else if a == 70000 {
	io.println("d") // expect: d
} else if a - 2 == 69998 {
	io.println("e")
}

io.println("final") // expect: final
