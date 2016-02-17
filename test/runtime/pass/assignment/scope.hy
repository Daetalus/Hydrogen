import "io"

let a = 3
io.println(a) // expect: 3

{
	let b = 4
	io.println(b) // expect: 4
}

{
	let b = 10
	io.println(b) // expect: 10
}

let b = 99
io.println(b) // expect: 99

{
	let c = 1
	io.println(c) // expect: 1

	{
		let d = 2
		io.println(d) // expect: 2
	}

	let d = 22
	io.println(d) // expect: 22
}
