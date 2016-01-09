
import "io"

fn fib(n) {
	if n <= 2 {
		return 1
	} else {
		return fib(n - 1) + fib(n - 2)
	}
}

io.println(fib(35))
