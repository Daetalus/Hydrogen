// true|false|true|true|true|false|true|true|true|false|true|false|false|true

import "err"


// Local, integer

let a = 3
let b = a < 4
err.println(b)

b = a > 4
err.println(b)

b = a == 3
err.println(b)

b = 9 != a
err.println(b)

b = a >= 3
err.println(b)

b = a <= 2
err.println(b)


// Local, number

a = 10.2
b = 10.3 >= a
err.println(b)

b = a >= 4.31415
err.println(b)

b = 10.3 != a
err.println(b)

b = a == 11.8847
err.println(b)


// Local, string

a = "hello"
b = "test"
err.println(a != b)
err.println(a == b)
err.println(a == "hell")
err.println(a != "hell")
