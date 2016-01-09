
num = 1
lastPrime = 0

while num < 50000:
    num += 1

    test = 1
    ok = True

    while test < num - 1:
        test += 1

        if num % test == 0:
            ok = False
            break

    if ok:
        lastPrime = num

print(lastPrime)
