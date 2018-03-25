sector_factor = { 1, 0.8, 0.3, 0, 0, 0.3, 0.8, 1 }
max_distance = 150
local sectors = 8
local q = {}

function step(food, segments)
	for i = 1, sectors do
		q[i] = {}
		q[i].sum = 0
		q[i].nearest_dist = 10000
		q[i].nearest_dir = 3.14
		q[i].factor = 0
		q[i].dist_to_collition = math.huge
	end

	local items_processed = 0
	local items_in_dist = 0
	for i, item in ipairs(food) do
		items_processed = items_processed + 1
		if item.dist < max_distance then
			items_in_dist = items_in_dist + 1
			local qi = 1 + math.floor((sectors/2) * item.d / math.pi)
			q[qi].sum = q[qi].sum + sector_factor[qi] * item.v
			if item.dist < q[qi].nearest_dist then
				q[qi].nearest_dist = item.dist
				q[qi].nearest_dir = item.d
			end
		end
	end

	for i, item in ipairs(segments) do
		if item.bot ~= self.id then
			local qi = 1 + math.floor((sectors/2) * item.d / math.pi)
			local dist_to_collition = item.dist - item.r - self.r
			if dist_to_collition < q[qi].dist_to_collition then
				q[qi].dist_to_collition = dist_to_collition
			end
		end
	end

	local target_q = nil
	local safest_q = nil
	local target_sum = 0
	local safest_dist = 0

	for i = 1, sectors do
		if (q[i].sum > target_sum) then
			target_q = i
			target_sum = q[i].sum
		end
		if (q[i].dist_to_collition > safest_dist) then
			safest_q = i
			safest_dist = q[i].dist_to_collition
		end
	end

	if target_q == nil then
		return 0.01
	end

	if is_dangerous(target_q) then
		target_q = safest_q
	end

	if target_q == nil then
		return 0.01
	end

	return q[target_q].nearest_dir
end

function is_dangerous(x)
	if q[x].dist_to_collition < 100 then return true end
	if q[neighbour(x, -1)].dist_to_collition < 80 then return true end
	if q[neighbour(x, 1)].dist_to_collition < 80 then return true end
	return false
end

function neighbour(x, dir)
	n = x + dir
	if (n>8) then return 1 end
	if (n<1) then return 8 end
	return n
end
