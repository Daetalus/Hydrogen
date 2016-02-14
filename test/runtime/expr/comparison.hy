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


// Complex

a = 1
io.println(a == a + 1) // expect: false
io.println(a + 1 != a + 2) // expect: true
io.println(a + 2 == a * 2) // expect: true

b = 2
io.println(a - 1 == b) // expect: false
io.println(a == b - 1) // expect: true

io.println((a == 1) == true) // expect: true
io.println((a == 5) == false) // expect: true
io.println((a == 1) == (b == 2)) // expect: true
io.println((a < -4) == (b > 5)) // expect: true

io.println(((a == 1) == true) == ((b == 3) == false)) // expect: true

io.println(a && b) // expect: true
io.println(a || b) // expect: true
io.println(a == 3 || b) // expect: true
io.println(a && b == -9) // expect: false
