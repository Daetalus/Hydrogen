
fn test() {
	let i = 10
	return fn() {
		return i
	}
}

let c = 1
let a = 3
let b = test()

assert(b() == 10)
