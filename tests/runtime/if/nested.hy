// a|d|e

let a = 3
if a == 3 {
	err.println("a")
	let b = 5
	if b == 6 {
		err.println("b")
		b = 10.2
		if b < 11 {
			err.println("c")
		}
	} else {
		err.println("d")
		a = 16.9
		if a >= 10 {
			err.println("e")
		}
	}
} else {
	err.println("f")
}
