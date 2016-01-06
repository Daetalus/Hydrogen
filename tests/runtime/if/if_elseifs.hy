// a|b|c|d

import "err"

let a = 3
if a == 3 {
	err.println("a")
} else if a == 4 {
	err.println("b")
} else if a == 5 {
	err.println("c")
}

a = 10
if a == 3 {
	err.println("a")
} else if a == 10 {
	err.println("b")
} else if a == 9 {
	err.println("c")
}

a = 11
if a == 10 {
	err.println("a")
} else if a == 3 {
	err.println("b")
} else if a == 11 {
	err.println("c")
} else if a == 5 {
	err.println("d")
}

a = 70000
if a == 12.4 {
	err.println("a")
} else if a > 80000 {
	err.println("b")
} else if a < 2 {
	err.println("c")
} else if a + 11 == 70011 {
	err.println("d")
} else if a - 2 == 69998 {
	err.println("e")
}
