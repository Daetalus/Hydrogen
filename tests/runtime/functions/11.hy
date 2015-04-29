
fn fib(n) {
	if n <= 1 {
		return 1
	}

	return fib(n - 1) + fib(n - 2)
}

assert(fib(1) == 1)
assert(fib(2) == 2)
assert(fib(3) == 3)
assert(fib(4) == 5)
assert(fib(5) == 8)
assert(fib(6) == 13)
assert(fib(7) == 21)
