let a = 0
let b = 10

print(b)
print(a < b)

while a < b {
	a += 2
	print(a)
}

print(a)
assert(a == 10)
assert(b == 10)
