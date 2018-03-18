local x = 0
local phi0 = math.random() - 0.5

function run(current_heading)
    x = x+0.03
	return current_heading + phi0 + math.sin(x)
end
