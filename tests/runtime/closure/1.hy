
let a = 3

fn test() {
	a = 4
}

assert(a == 3)
test()
assert(a == 4)
