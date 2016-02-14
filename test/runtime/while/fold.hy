import "io"

let i = 0
while true {
	io.println(i)
	if i >= 3 {
		break
	}
	i = i + 1
}

// expect: 0
// expect: 1
// expect: 2
// expect: 3


i = 0
while false {
	io.println("hello")
}

while 3 == 5 {
	io.println("anyone?")
}
