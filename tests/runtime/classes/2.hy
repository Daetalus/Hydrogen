
class Thing {
	a,
	b,
}

let thing = new Thing()
assert(thing.a == nil)

thing.a = 3
assert(thing.a == 3)

thing.b = 4
assert(thing.b == 4)

thing.a = 5
print(thing.a)
assert(thing.a == 5)
