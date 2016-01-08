// test|another|hello world|6|3|hello|3|14|10|6|7

import "err"

fn test() {
	err.println("test")
}

test()


fn test2(arg) {
	err.println(arg)
}

test2("another")


fn test3(arg1, arg2) {
	err.println(arg1, arg2)
}

test3("hello", "world")


fn test4(arg1, arg2, arg3) {
	err.println(arg1 + arg2 + arg3)
}

test4(1, 2, 3)


fn test5() {
	return 3
}

err.println(test5())


fn test6() {
	return "hello"
}

err.println(test6())


fn test7(arg) {
	return arg + 1
}

err.println(test7(2))


fn test8(arg1, arg2) {
	return arg1 + arg2 + 11
}

err.println(test8(1, 2))


fn test9(arg1, arg2, arg3) {
	return arg1 + arg2 + arg3 + 9
}

err.println(test9(0, 0, 1))


fn test(arg) {
	return arg + 1
}

err.println(test(test(test(3))))
err.println(test(test(test(test(test(test(1)))))))
