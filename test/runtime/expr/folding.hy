import "io"

io.println(3 + 4) // expect: 7
io.println(3 * 4 + 9) // expect: 21
io.println((3 + 4) * 5) // expect: 35
io.println(((3 * (1 + 2)) + 1) * 2) // expect: 20
io.println(-3 - 1) // expect: -4
io.println(-100 * 2) // expect: -200

io.println(100 == 100) // expect: true
io.println(3 == 4) // expect: false
io.println(100 != 99) // expect: true
io.println(55 != 55) // expect: false
io.println(3 < 4) // expect: true
io.println(5 < 4) // expect: false
io.println(4 < 4) // expect: false
io.println(123 <= 124) // expect: true
io.println(5 <= 5) // expect: true
io.println(123 <= -99) // expect: false
io.println(5 > 3) // expect: true
io.println(6 > 2) // expect: false
io.println(3 > 3) // expect: false
io.println(3 >= 1) // expect: true
io.println(3 >= 3) // expect: true
io.println(3 >= 99) // expect: false

io.println(3.141592 == 3.141592) // expect: true
io.println(3.01 != 3.01) // expect: false

io.println(true && true) // expect: true
io.println(false && true) // expect: false
io.println(true && false) // expect: false
io.println(false && false) // expect: false

io.println(true || true) // expect: true
io.println(true || false) // expect: true
io.println(false || true) // expect: true
io.println(false || false) // expect: false

io.println((true && true) || false) // expect: true
io.println(false || (true && false)) // expect: false
io.println((true && false) || (true && true && true)) // expect: true
io.println((3 == 5 && 1 == 1) || 1 == 5 || 3 >= 3) // expect: true
