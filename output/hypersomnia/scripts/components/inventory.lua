components.inventory = inherits_from()

function components.inventory:constructor(init_table)
	self.available_items = {}
	
	rewrite(self, init_table)
end