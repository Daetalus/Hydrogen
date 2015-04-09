
let n = 10
let total = 0
let j = 0
while j < n {
	let current = 0
	let previous = 1
	let i = 0

	while i < 10 {
		let temp = current
		current += previous
		previous = temp
		total += current
		print(current)
		print(total)
		i += 1
	}

	j += 1
}

print(total)
assert(total == n * 143)
