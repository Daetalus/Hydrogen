import "io"

fn test0() {}

io.println(test0()) // expect: nil


fn test1() {
	io.println("test")
}

test1() // expect: test


fn test2(arg) {
	io.println(arg)
}

test2("another") // expect: another


fn test3(arg1, arg2) {
	io.println(arg1, arg2)
}

test3("hello", "world") // expect: hello world


fn test4(arg1, arg2, arg3) {
	io.println(arg1 + arg2 + arg3)
}

test4(1, 2, 3) // expect: 6


fn test5() {
	return 3
}

io.println(test5()) // expect: 3


fn test6() {
	return "hello"
}

io.println(test6()) // expect: hello


fn test7(arg) {
	return arg + 1
}

io.println(test7(2)) // expect: 3


fn test8(arg1, arg2) {
	return arg1 + arg2 + 11
}

io.println(test8(1, 2)) // expect: 14


fn test9(arg1, arg2, arg3) {
	return arg1 + arg2 + arg3 + 9
}

io.println(test9(0, 0, 1)) // expect: 10


fn test10(arg) {
	return arg + 1
}

io.println(test10(test10(test10(3)))) // expect: 6
io.println(test10(test10(test10(test10(test10(test10(1))))))) // expect: 7


fn test11(arg) {
	return arg(3)
}

io.println(test11(fn(arg) {
	return arg + 1
})) // expect: 4


fn early_return(arg) {
	if arg == 3 {
		return 4
	}
	io.println("thing")
	return 3
}

io.println(early_return(3)) // expect: 4
io.println(early_return(4))
// expect: thing
// expect: 3


fn nested1() {
	fn nested2() {
		return fn(arg3) {
			return arg3 + 1
		}
	}
	return nested2()
}

let fn_thing = nested1()
io.println(fn_thing(3)) // expect: 4
io.println(fn_thing(5)) // expect: 6


fn return_nil() {
	let a = 3
}

let a = return_nil()
io.println(a) // expect: nil
io.println(return_nil()) // expect: nil
