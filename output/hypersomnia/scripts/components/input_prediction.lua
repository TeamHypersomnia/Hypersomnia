components.input_prediction = inherits_from()

function components.input_prediction:constructor(init_table) 
	self.state_history = {}
	
	self.accept_divergence_in_radius = 10
	self.simulation_entity = init_table.simulation_entity
end