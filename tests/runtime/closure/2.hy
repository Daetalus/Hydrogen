
fn iter() {
	let a = 3
	return fn() {
		return a + 1
	}
}

let the_fn = iter()
assert(the_fn() == 4)
assert(the_fn() == 4)
