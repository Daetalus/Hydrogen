
import "io"

let a = [1, 2, 3, 4]
io.println(a) // expect: [1, 2, 3, 4]
io.println(a[0]) // expect: 1
io.println(a[1]) // expect: 2
io.println(a[2]) // expect: 3

let b = 3
io.println(a[b]) // expect: 4
io.println(a[b - 1]) // expect: 3
io.println(a[(b - 1) / 2]) // expect: 2

let c = [1, [1, 2, [1]]]
io.println(c[0]) // expect: 1
io.println(c[1]) // expect: [1, 2, [1]]
io.println(c[1][0]) // expect: 1
io.println(c[1][1]) // expect: 2
io.println(c[1][2]) // expect: [1]
io.println(c[1][2][0]) // expect: 1

io.println(a[c[0]]) // expect: 2
io.prinltn(a[c[1][2][0]]) // expect: 2
