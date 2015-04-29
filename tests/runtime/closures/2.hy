
fn test() {
	return fn() {
		return 10
	}
}

let func = test()
assert(func() == 10)
