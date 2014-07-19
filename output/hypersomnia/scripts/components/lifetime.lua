components.lifetime = inherits_from()

function components.lifetime:constructor(init_table)
	self.max_lifetime_ms = -1
	self.max_distance = -1
	
	rewrite(self, init_table)
	
	
	self.current_lifetime = timer()
end