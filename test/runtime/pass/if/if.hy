import "io"

let a = 3
if a == 3 {
	io.println("a") // expect: a
}

a = "hello"
if a == "hello" {
	io.println("b") // expect: b
}

if a != "hello" {
	io.println("c")
}

if a != "t" {
	io.println("d") // expect: d
}

a = false
if a {
	io.println("e")
}

a = nil
if a {
	io.println("f")
}

a = 10.4
if a {
	io.println("g") // expect: g
}

a = "test"
if a {
	io.println("h") // expect: h
}

io.println("final") // expect: final
