
let total = 0
let i = 0
loop {
	if i >= 10 {
		break
	}

	let j = 0
	loop {
		if j >= 20 {
			break
		}

		let n = 1
		total += n
		j += 1
	}

	i += 1
}

assert(total == 200)
