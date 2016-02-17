import "io"

if true {
	let a = 3
	io.println(a) // expect: 3
}

let a = 4
io.println(a) // expect: 4

if false {
	io.println("hello")
}

io.println("after") // expect: after

if false {
	io.println("something")
} else {
	io.println("else") // expect: else
}

if true {
	io.println("yes") // expect: yes
} else {
	io.println("else")
}

if true {
	io.println("a") // expect: a
} else if true {
	io.println("b")
} else {
	io.println("c")
}

if 3 == 4 {
	io.println("c")
} else if true {
	io.println("a") // expect: a
} else {
	io.println("b")
}

if 3 == 4 {
	io.println("c")
} else if false && true {
	io.println("b")
} else {
	io.println("a") // expect: a
}
