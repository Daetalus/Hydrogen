
let total = 0
let a = 10
loop {
	if a <= 0 {
		break
	}

	let b = 2 * a
	total += b
	a -= 1
}

assert(total == 110)
