
local num = 1
local lastPrime = 0

while num < 50000 do
	num = num + 1

	local test = 1
	local ok = true

	while test < num - 1 do
		test = test + 1

		if num % test == 0 then
			ok = false
			break
		end
	end

	if ok then
		lastPrime = num
	end
end

print(lastPrime)
