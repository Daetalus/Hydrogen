
fn iter() {
	let i = 0
	return fn() {
		i += 1
		return i
	}
}

let iterator = iter()
assert(iterator() == 1)
assert(iterator() == 2)
assert(iterator() == 3)
assert(iterator() == 4)
assert(iterator() == 5)
assert(iterator() == 6)
