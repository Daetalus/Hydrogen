
let k = 1234.56789
let x = k

let i = 0
while i < 99 {
	x = (x + k / x) / 2
	print(x)
	i += 1
}
