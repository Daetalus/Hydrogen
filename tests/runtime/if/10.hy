if 1 + 2 == 4 && 3 + 8 == 11 {
	assert(false)
} else if 3 + 4 == 5 || 19 < 10 {
	assert(false)
} else if 11 == 12 || (3 == 4 && 5 == 5) {
	assert(false)
} else if 3 * 4 + 16 - 32 / 2 == 12 {
	assert(true)
} else if false && true {
	assert(false)
} else {
	assert(false)
}
