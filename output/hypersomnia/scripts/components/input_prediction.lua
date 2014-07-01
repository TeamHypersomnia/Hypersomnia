components.input_prediction = inherits_from()

function components.input_prediction:constructor(init_table) 
	self.state_history = {}
	
	self.simulation_entity = init_table.simulation_entity
end