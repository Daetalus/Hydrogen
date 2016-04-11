
import "io"

let a = [1, 2, 3, 4]

io.println(a[0]) // expect: 1
a[0] = 2
io.println(a[0]) // expect: 2

io.println(a[1]) // expect: 2
a[1] = "hello"
io.println(a[1]) // expect: hello
a[1] = true
io.println(a[1]) // expect: true

a[3] = a[2] + 10
io.println(a[3]) // expect: 13


let b = [1, [1, 2, [3]]]

io.println(b[1][0]) // expect: 1
b[1][0] = 2
io.println(b[1][0]) // expect: 2

io.println(b[1][2][0]) // expect: 3
b[1][2][0] = 10
io.println(b[1][2][0]) // expect: 10
