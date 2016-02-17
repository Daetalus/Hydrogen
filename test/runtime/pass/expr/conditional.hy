import "io"

let a = 3
let b = 4
let c = 5
let d = 9
let e = 123
io.println(a == 3 && b == 4) // expect: true
io.println(a == 4 && b == 4) // expect: false
io.println(a == 4 && b == 5) // expect: false
io.println(a > 2 && b < 4) // expect: false
io.println(a >= 2 && b <= 4) // expect: true

io.println(a == 3 && b == 4 && c == 5) // expect: true
io.println(a >= 3 && b <= 5 && c < 10) // expect: true
io.println(a == 3 && b == 4 && c < 4) // expect: false
io.println(a == 3 && b == 9 && c == 5) // expect: false
io.println(a == 100 && b == 4 && c > 2) // expect: false

io.println(a == 3 || b == 4) // expect: true
io.println(a == 4 || b == 4) // expect: true
io.println(a == 4 || b == 5) // expect: false

io.println(a == 3 || b == 4 || c == 5) // expect: true
io.println(a == 3 || b == 4 || c == 10) // expect: true
io.println(a == 3 || b == 100 || c == 5) // expect: true
io.println(a == 9 || b == 4 || c == 5) // expect: true
io.println(a == 123 || b == 64 || c == 100) // expect: false

io.println((a == 3 && b == 4) || c == 100) // expect: true
io.println((a == 5 && b == 4) || c == 5) // expect: true
io.println(c == 5 || (a == 5 && b == 4)) // expect: true
io.println(c == 5 || (a == 3 && b == 4)) // expect: true
io.println(c == 100 || (a == 123 && b == 88)) // expect: false
io.println((c == 5 || b == 123) && a == 3) // expect: true
io.println((c == 5 || b == 123) && a == 4) // expect: false
io.println((c == 5 || b == 4) && a == 123) // expect: false
io.println((c == 5 || b == 4) && a == 3 && b == 4) // expect: true
io.println((c == 5 || b == 4) && a == 3 && b == 7) // expect: false
io.println((c == 5 || b == 123) && a == 3 && (d == 9 || e == 1)) // expect: true
io.println((c == 1 || b == 4 || e == 123) && a == 3 && (d == 1 || e == 3)) // expect: false
io.println(a == 3 && (b == 4 || e == 51) && d == 9) // expect: true
io.println(a == 5 && b == 1 && (d == 9 || e == 123)) // expect: false
