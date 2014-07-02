components.input_prediction = inherits_from()

function components.input_prediction:constructor(init_table) 
	self.state_history = {}
	
	self.first_state = 1
	self.count = 0
	
	self.accept_divergence_in_radius = 10
	self.simulation_entity = init_table.simulation_entity
end