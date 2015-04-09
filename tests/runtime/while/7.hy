
let total = 0
let j = 0
while j < 100 {
	let current = 1
	let previous = 0
	let i = 0

	while i < 10 {
		let temp = current
		current += previous
		previous = temp
		total += current
		i += 1
	}

	j += 1
}

assert(total == 231 * 100)
