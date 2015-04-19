
let a = 3

let b = fn() {
	a += 1
}

let c = fn() {
	a += 2
}

assert(a == 3)
b()
assert(a == 4)
c()
assert(a == 6)
b()
c()
assert(a == 9)
c()
assert(a == 11)
