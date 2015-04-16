
fn iter() {
	let i = 0
	return fn() {
		return i + 1
	}
}


fn iter2() {
	let i = 10
	return fn() {
		return i + 1
	}
}


let fn1 = iter()
let fn2 = iter2()

assert(fn1() == 1)
assert(fn2() == 11)
