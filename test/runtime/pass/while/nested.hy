import "io"

let a = 0
while a < 2 {
	let b = 0
	while b < 2 {
		io.println(b)
		b = b + 1
	}
	a = a + 1
}

// expect: 0
// expect: 1
// expect: 0
// expect: 1
