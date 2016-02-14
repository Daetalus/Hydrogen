import "io"

let a = 3
loop {
	if a > 5 {
		break
	}

	io.println(a)
	a = a + 1
}

// expect: 3
// expect: 4
// expect: 5
