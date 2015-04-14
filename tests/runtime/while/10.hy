
let total = 0

let i = 0
while i < 10 {
	let j = 0
	while j < 20 {
		if j >= 10 {
			break
		}

		total += 1
		j += 1
	}

	i += 1
}

assert(total == 100)
