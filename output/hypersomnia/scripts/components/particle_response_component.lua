components.particle_response = inherits_from()

function components.particle_response:constructor(init_table)
	recursive_write(self, init_table)
end