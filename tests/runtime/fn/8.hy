
fn test(arg) {
	return arg + 1
}


fn hello(arg) {
	return test(arg + 1)
}


assert(hello(1) == 3)
