input_prediction_system = inherits_from (processing_system)

function input_prediction_system:constructor(simulation_world, substepping_world) 
	self.simulation_world = simulation_world
	self.substepping_world = substepping_world
	
	table.insert(self.substepping_world.substep_callbacks, function () self:substep_callback(self.substepping_world) end)
	
	processing_system.constructor(self)
end

function input_prediction_system:get_required_components()
	return { "input_prediction", "cpp_entity" }
end

function input_prediction_system:substep_callback(owner_world)
	for i=1, #self.targets do
		local prediction = self.targets[i].input_prediction
		local movement = self.targets[i].cpp_entity.movement
		
		local history_entry = { 
			position = self.targets[i].cpp_entity.physics.body:GetPosition(),
			moving_left = movement.moving_left,
			moving_right = movement.moving_right,
			moving_forward = movement.moving_forward,
			moving_backward = movement.moving_backward
		}
		
		
		self.substepping_world.render_system:clear_non_cleared_lines()
		self.substepping_world.render_system:push_non_cleared_line(debug_line(vec2(history_entry.position.x*50, history_entry.position.y*50), vec2(0, 0), rgba(255, 255, 255, 255)))
		
		table.insert(prediction.state_history, history_entry)
		
		if #prediction.state_history > 60 then
			table.remove(prediction.state_history, 1)
		end
		
		print(table.inspect(history_entry))
	end
end

function input_prediction_system:apply_correction(entity, new_position) 


end