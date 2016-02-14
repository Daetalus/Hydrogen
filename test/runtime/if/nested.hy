import "io"

let a = 3
if a == 3 {
	io.println("a") // expect: a
	let b = 5
	if b == 6 {
		io.println("b")
		b = 10.2
		if b < 11 {
			io.println("c")
		}
	} else {
		io.println("d") // expect: d
		a = 16.9
		if a >= 10 {
			io.println("e") // expect: e
		}
	}

	io.println("after") // expect: after
} else {
	io.println("f")
}

io.println("final") // expect: final
