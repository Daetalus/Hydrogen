
let current = 1
let previous = 1
let i = 0

while i < 100 {
	let temp = current
	current += previous
	previous = temp
	i += 1
}

print(current)
