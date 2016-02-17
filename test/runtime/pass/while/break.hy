import "io"

let n = 0
while n < 100 {
	if n >= 5 {
		break
	}
	io.println(n)
	n = n + 1
}

// expect: 0
// expect: 1
// expect: 2
// expect: 3
// expect: 4

io.println("after") // expect: after


n = 1
let a = 3
while a == 3 {
	if n % 5 == 0 {
		break
	}
	n = n + 1
	io.println("yes")
}

// expect: yes
// expect: yes
// expect: yes
// expect: yes
