function step(food)

	local sectors = 16;
	local q = {}
	local largest_q = 0;
	local largest_sum = 0;

	for i = 1, sectors do
		q[i] = {}
		q[i].sum = 0
		q[i].nearest_dist = 10000
		q[i].nearest_dir = 0
	end

	for i, item in ipairs(food) do
		local qi = 1 + math.floor((sectors/2) * item.d / math.pi)

		q[qi].sum =q[qi].sum + item.v
		if (q[qi].sum > largest_sum) then
			largest_sum = q[qi].sum
			largest_q = qi
		end

		if (item.dist < q[qi].nearest_dist) then
			q[qi].nearest_dist = item.dist
			q[qi].nearest_dir = item.d
		end
	end

	if (largest_sum<10) then
		return 0
	end

	for i = 1, sectors do
		log(string.format("q=%d sum=%.2f dist=%.2f dir=%.2f", i, q[i].sum, q[i].nearest_dist, q[i].nearest_dir))
	end

	log(string.format("going to dir %.2f", q[largest_q].nearest_dir))
	return q[largest_q].nearest_dir
end
