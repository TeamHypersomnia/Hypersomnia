components.health = inherits_from()
components.health.max_hp = 100

function components.health:constructor(init_table) 
	self.hp = 100
	
	rewrite(self, init_table)
end