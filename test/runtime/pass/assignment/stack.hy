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

	b = 1e3
	io.println(b) // expect: 1000

	b = 1e-3
	io.println(b) // expect: 0.001

	b = 24e2
	io.println(b) // expect: 2400
}
