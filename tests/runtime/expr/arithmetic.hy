// 7|-1|12|0.75|1|7|1|6|0.3|1|9.5|4.9|10.92|2.08

import "err"


// Local, local

let a = 3
let c = 4
let b = a + c
err.println(b)

b = a - c
err.println(b)

b = a * c
err.println(b)

b = a / c
err.println(b)

a = 9
b = a % c
err.println(b)


// Local, integer

a = 3
b = a + 4
err.println(b)

b = a - 2
err.println(b)

b = a * 2
err.println(b)

b = a / 10
err.println(b)

b = a % 2
err.println(b)


// Local, number

let d = 5.2
b = d + 4.3
err.println(b)

b = d - 0.3
err.println(b)

b = d * 2.1
err.println(b)

b = d / 2.5
err.println(b)


// Negatives
// TODO
