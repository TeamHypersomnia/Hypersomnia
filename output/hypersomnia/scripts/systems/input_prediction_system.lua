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
		local position_meters = self.targets[i].cpp_entity.physics.body:GetPosition()
		
		local history_entry = { 
			position = to_pixels(position_meters),
			moving_left = movement.moving_left,
			moving_right = movement.moving_right,
			moving_forward = movement.moving_forward,
			moving_backward = movement.moving_backward
		}
		clearl()
		
		table.insert(prediction.state_history, history_entry)
		
		--if #prediction.state_history > 60 then
		--	table.remove(prediction.state_history, 1)
		--end
		
		for j=1, #prediction.state_history do
			debugl(rgba(255, 255, 255, 255), prediction.state_history[j].position)
		end
	end
end

function input_prediction_system:apply_correction(entity, new_position) 


end

function input_prediction_system:update()
	for i=1, #self.targets do
		local prediction = self.targets[i].input_prediction
		local movement_sync = self.targets[i].synchronization.modules.movement
		
		debugl(rgba(255, 0, 0, 255), to_pixels(movement_sync.position), to_pixels(movement_sync.position) + to_pixels(movement_sync.velocity) )
		
		-- write the most recent prediction and request correction
		local prediction_request = BitStream()
		prediction_request:name_property("CLIENT_PREDICTION")
		prediction_request:WriteByte(protocol.messages.CLIENT_PREDICTION)
		prediction_request:name_property("input_sequence")
		-- we're already ahead of one prediction
		-- as substep callback is called before b2World::Step
		prediction_request:WriteUshort(#prediction.state_history+1)
		-- write our current position
		prediction_request:name_property("predicted_pos")
		prediction_request:Writeb2Vec2(self.targets[i].cpp_entity.physics.body:GetPosition())
		
		self.owner_entity_system.all_systems["client"].net_channel.unreliable_buf:WriteBitstream(prediction_request)
		
		for s=1, #prediction.state_history do
			
		end
	end
end

