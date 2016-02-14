import "io"

io.println(-3) // expect: -3

let a = 3
io.println(-a) // expect: -3

let b = 4
io.println(-(a + b)) // expect: -7
io.println(-a - b) // expect: -7
io.println(-a + b) // expect: 1

let c = 5
io.println(-a + b + c) // expect: 6
io.println(-a * b) // expect: -12
io.println(-(a * b)) // expect: -12
