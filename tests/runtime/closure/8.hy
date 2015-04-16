
fn iter(i) {
	return fn() {
		i += 1
		return i
	}
}

let iterator = iter(0)
assert(iterator() == 1)
assert(iterator() == 2)
assert(iterator() == 3)
assert(iterator() == 4)
assert(iterator() == 5)
