
let a = 3

fn test() {
	if a > 10 {
		return
	}

	a += 1
	test()
}


test()
assert(a == 11)
