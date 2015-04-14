let a = 10
let total = 0
while a > 0 {
	let b = 2 * a
	total += b
	if a == 5 {
		let temp = 3
		break
	}

	a -= 1
}

assert(a == 5)
assert(total == 90)
