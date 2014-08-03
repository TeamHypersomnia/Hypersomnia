components.health = inherits_from()

function components.health:constructor(init_table) 
	self.hp = 80
	
	rewrite(self, init_table)
end