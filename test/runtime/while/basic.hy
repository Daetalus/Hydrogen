// 0|1|2|3|4|100|99|98|97|96|1|1|2|3|5|8|13|21|34|55

import "err"

let n = 0
while n < 5 {
	err.println(n)
	n = n + 1
}


n = 100
while n > 95 {
	err.println(n)
	n = n - 1
}


let previous = 0
let current = 1
n = 0
while n < 10 {
	err.println(current)
	let temp = previous
	previous = current
	current = temp + current
	n = n + 1
}


let a = 3
while a == a + 1 {
	err.println("something")
}
