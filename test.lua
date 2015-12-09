
local function test()
	local a = 0
	return function()
		local b = 2
		a = 2 + b + a + 1
		return a
	end
end

local b = test()
b()
