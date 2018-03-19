sector_factor = { 1, 0.8, 0.3, 0, 0, 0.3, 0.8, 1 }
max_distance = 150

function step(food)
	local largest_q = 0
	local largest_sum = 0
	local sectors = 8

	local q = {}
	for i = 1, sectors do
		q[i] = {}
		q[i].sum = 0
		q[i].nearest_dist = 10000
		q[i].nearest_dir = 3.14
		q[i].factor = 0
	end

	local items_processed = 0
	local items_in_dist = 0

	for i, item in ipairs(food) do
		items_processed = items_processed + 1
		if item.dist < max_distance then
			items_in_dist = items_in_dist + 1

			local qi = 1 + math.floor((sectors/2) * item.d / math.pi)
			if (qi < 1) or (qi > sectors) then
				log(string.format("qi<>sectors: sectors=%d d=%.2f", sectors, item.d));
			end

			q[qi].sum = q[qi].sum + sector_factor[qi] * item.v
			if (q[qi].sum > largest_sum) then
				largest_sum = q[qi].sum
				largest_q = qi
			end

			if (item.v > 1) then
				if (item.dist < q[qi].nearest_dist) then
					q[qi].nearest_dist = item.dist
					q[qi].nearest_dir = item.d
				end
			end
		end
	end

	if largest_q == 0 then
		log(string.format("no food found?! processed=%d in dist: %d", items_processed, items_in_dist))
		return 0.01
	end

	return q[largest_q].nearest_dir
end
