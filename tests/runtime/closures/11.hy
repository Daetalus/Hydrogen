
fn iter() {
	let a = 3
	return fn() {
		a += 1
		return fn() {
			return a + 1
		}
	}
}

let a = iter()
let b = a()
assert(b() == 5)
assert(b() == 5)
let c = a()
assert(c() == 6)
assert(c() == 6)
