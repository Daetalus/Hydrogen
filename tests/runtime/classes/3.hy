
class Test {
	a
}

let thing1 = new Test()
let thing2 = new Test()
let thing3 = new Test()

thing1.a = thing2
thing2.a = thing3
thing3.a = 4

assert(thing1.a.a.a == 4)
