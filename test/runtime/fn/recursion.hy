// 6|120|5040|0|1|1|2|3|5|8|13

import "err"

fn factorial(n) {
	if n <= 1 {
		return 1
	} else {
		return n * factorial(n - 1)
	}
}

err.println(factorial(3))
err.println(factorial(5))
err.println(factorial(7))


fn fib(n) {
	if n == 1 {
		return 0
	} else if n == 2 {
		return 1
	} else {
		return fib(n - 1) + fib(n - 2)
	}
}

err.println(fib(1))
err.println(fib(2))
err.println(fib(3))
err.println(fib(4))
err.println(fib(5))
err.println(fib(6))
err.println(fib(7))
err.println(fib(8))
