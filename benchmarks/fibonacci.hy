
let n = 10
let total = 0
let j = -100
while j < 0 {
	let current = 0
	let previous = 1
	let i = 0

	while i < 10 {
		let temp = current
		current += previous
		previous = temp
		print(current)
		i += 1
	}

	print("j is " + j)
	j += 1
}
