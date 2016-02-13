import "io"

{
	let a = 3
	io.println(a) // expect: 3

	let b = 3.1415926535
	io.println(b) // expect: 3.1415926535

	let c = "hello"
	io.println(c) // expect: hello

	b = c
	io.println(b) // expect: hello

	b = nil
	io.println(b) // expect: nil
}
