import "io"

fn test() {
	io.println("test")
}

test() // expect: test


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
