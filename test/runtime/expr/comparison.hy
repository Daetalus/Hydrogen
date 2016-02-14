import "io"


// Local, integer

let a = 3
let b = a < 4
io.println(b) // expect: true

b = a > 4
io.println(b) // expect: false

b = a == 3
io.println(b) // expect: true

b = 9 != a
io.println(b) // expect: true

b = a >= 3
io.println(b) // expect: true

b = a <= 2
io.println(b) // expect: false


// Local, number

a = 10.2
b = 10.3 >= a
io.println(b) // expect: true

b = a >= 4.31415
io.println(b) // expect: true

b = 10.3 != a
io.println(b) // expect: true

b = a == 11.8847
io.println(b) // expect: false


// Local, string

a = "hello"
b = "test"
io.println(a != b) // expect: true
io.println(a == b) // expect: false
io.println(a == "hell") // expect: false
io.println(a != "hell") // expect: true
