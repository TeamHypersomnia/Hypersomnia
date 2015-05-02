components.light = inherits_from()

function components.light:constructor(init_table)
	self.attenuation_variations = {
		{
			value = 0,
			min_value = -0.3,
			max_value = 0.0,
			change_speed = 0.8/5
		},

		{
			value = 0,
			min_value = -0.00001,
			max_value = 0.00002,
			change_speed = 0.0002/5
		},

		{
			value = 0,
			min_value = -0.00005,
			max_value = 0.00030,
			change_speed = 0.0003/12
		},

		-- light position variation
		{
			value = 0,
			min_value = -10,
			max_value = 10,
			change_speed = 50
		},

		{
			value = 0,
			min_value = -10,
			max_value = 10,
			change_speed = 50
		}
	}

	recursive_write(self, init_table)
end