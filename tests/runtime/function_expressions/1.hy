
let a = 1

fn test() {
	a = 3
}

assert(a == 1)
(test)()
assert(a == 3)
