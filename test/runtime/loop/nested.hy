import "io"

let a = 0
loop {
	if a >= 2 {
		break
	}

	let b = 0
	loop {
		if b >= 2 {
			break
		}

		io.println(b)
		b = b + 1
	}

	a = a + 1
}

// expect: 0
// expect: 1
// expect: 0
// expect: 1
