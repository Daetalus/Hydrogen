import "io"


// Local, local

let a = 3
let c = 4
let b = a + c
io.println(b) // expect: 7

b = a - c
io.println(b) // expect: -1

b = a * c
io.println(b) // expect: 12

b = a / c
io.println(b) // expect: 0.75

a = 9
b = a % c
io.println(b) // expect: 1


// Local, integer

a = 3
b = a + 4
io.println(b) // expect: 7

b = a - 2
io.println(b) // expect: 1

b = a * 2
io.println(b) // expect: 6

b = a / 10
io.println(b) // expect: 0.3

b = a % 2
io.println(b) // expect: 1


// Integer, local

a = 3
b = 4 + a
io.println(b) // expect: 7

b = 2 - a
io.println(b) // expect: -1

b = 5 * a
io.println(b) // expect: 15

b = 24 / a
io.println(b) // expect: 8

b = 5 % a
io.println(b) // expect: 2


// Local, number

let d = 5.2
b = d + 4.3
io.println(b) // expect: 9.5

b = d - 0.3
io.println(b) // expect: 4.9

b = d * 2.1
io.println(b) // expect: 10.92

b = d / 2.5
io.println(b) // expect: 2.08


// Number, local and negatives

d = 6.3
b = -18.2 + d
io.println(b) // expect: -11.9

b = 4.2 - d
io.println(b) // expect: -2.1

b = 1.52 * -d
io.println(b) // expect: -9.576

b = -34.0326 / d
io.println(b) // expect: -5.402
