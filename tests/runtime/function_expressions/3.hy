
let test = 3

(fn(a, b) {
	assert(a == 1)
	assert(b == 2)
	assert(test == 3)
	test = 4
	assert(test == 4)
})(1, 2)

assert(test == 4)
