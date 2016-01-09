// hello|world|another test|3|6|3|3|4|4|120|720

import "err"

struct Test {
	a, b
}

let a = new Test()


fn (Test) test() {
	err.println("hello")
}

a.test()


fn (Test) test2(arg) {
	err.println(arg)
}

a.test2("world")


fn (Test) hello(arg1, arg2) {
	err.println(arg1, arg2)
}

a.hello("another", "test")


fn (Test) another() {
	return 3
}

err.println(a.another())


fn (Test) yetAnother(arg) {
	return arg + 3
}

err.println(a.yetAnother(3))


fn (Test) again(thing, thing2) {
	return thing + thing2
}

err.println(a.again(1, 2))


fn (Test) modify(arg) {
	self.a = arg
}

a.a = 3
err.println(a.a)
a.modify(4)
err.println(a.a)


fn (Test) ret() {
	return self.a
}

err.println(a.ret())


fn (Test) factorial(n) {
	if n <= 1 {
		return 1
	} else {
		return n * self.factorial(n - 1)
	}
}

err.println(a.factorial(5))
err.println(a.factorial(6))
