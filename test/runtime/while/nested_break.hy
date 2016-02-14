import "io"

let a = 0
while a < 100 {
	let b = 0
	while b < 100 {
		if b >= 2 {
			break
		}
		io.println(b)
		b = b + 1
	}

	a = a + 1
	if a >= 2 {
		break
	}
}

// expect: 0
// expect: 1
// expect: 0
// expect: 1
