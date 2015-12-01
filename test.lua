
local prec = {
	["+"] = 1,
	["*"] = 2,
}

function pop(expr)
	if #expr == 0 then
		return ""
	end

	local value = expr[1]
	table.remove(expr, 1)
	return value
end

function first(expr)
	if #expr == 0 then
		return ""
	else
		return expr[1]
	end
end

function emit(slot, operator, left, right)
	print(operator, slot, left, right)
end

function parse(expr, slot, limit)
	local left = pop(expr)
	local operator = first(expr)
	while operator ~= "" and prec[operator] > limit do
		pop(expr)

		local right = parse(expr, slot + 1, prec[operator])
		emit(slot, operator, left, right)

		left = right
		operator = expr[1]
	end
	return left
end

parse({"3", "*", "4", "+", "5"}, 0, 0)
