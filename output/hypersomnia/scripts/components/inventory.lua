components.inventory = inherits_from()

function components.inventory:constructor(init_table)
	self.available_items = {}
	self.pending_requests = {}
	
	rewrite(self, init_table)
end