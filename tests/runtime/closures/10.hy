
fn iter() {
	let a = 4
	return fn() {
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
assert(c() == 5)
assert(c() == 5)
