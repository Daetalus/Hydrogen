
import "io"

{
let num = 1
let lastPrime = 0

while num < 50000 {
	num = num + 1

	let test = 1
	let ok = true

	while test < num - 1 {
		test = test + 1

		if num % test == 0 {
			ok = false
			break
		}
	}

	if ok {
		lastPrime = num
	}
}

io.println(lastPrime)
}
