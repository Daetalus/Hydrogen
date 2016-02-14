import "io"

fn factorial(n) {
	if n <= 1 {
		return 1
	} else {
		return n * factorial(n - 1)
	}
}

io.println(factorial(3)) // expect: 6
io.println(factorial(5)) // expect: 120
io.println(factorial(7)) // expect: 5040


fn fib(n) {
	if n == 1 {
		return 0
	} else if n == 2 {
		return 1
	} else {
		return fib(n - 1) + fib(n - 2)
	}
}

io.println(fib(1)) // expect: 0
io.println(fib(2)) // expect: 1
io.println(fib(3)) // expect: 1
io.println(fib(4)) // expect: 2
io.println(fib(5)) // expect: 3
io.println(fib(6)) // expect: 5
io.println(fib(7)) // expect: 8
io.println(fib(8)) // expect: 13
