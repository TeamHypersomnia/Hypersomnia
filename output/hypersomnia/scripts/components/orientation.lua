components.orientation = inherits_from()

function components.orientation:constructor(init_table) 
	self.crosshair_entity = init_table.crosshair_entity
	self.receiver = init_table.receiver
	
	set_rate(self, "cmd", 10)
	
	self.last_pos = vec2(0, 0)
end