// 3|3.1415926535|hello|hello|nil

import "err"

{
	let a = 3
	err.println(a)

	let b = 3.1415926535
	err.println(b)

	let c = "hello"
	err.println(c)

	b = c
	err.println(b)

	b = nil
	err.println(b)
}
