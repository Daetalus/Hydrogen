import "io"

let n = 0
while n < 5 {
	io.println(n)
	n = n + 1
}

// expect: 0
// expect: 1
// expect: 2
// expect: 3
// expect: 4


n = 100
while n > 95 {
	io.println(n)
	n = n - 1
}

// expect: 100
// expect: 99
// expect: 98
// expect: 97
// expect: 96


let previous = 0
let current = 1
n = 0
while n < 10 {
	io.println(current)
	let temp = previous
	previous = current
	current = temp + current
	n = n + 1
}

// expect: 1
// expect: 1
// expect: 2
// expect: 3
// expect: 5
// expect: 8
// expect: 13
// expect: 21
// expect: 34
// expect: 55


let a = 3
while a == a + 1 {
	io.println("something")
}
