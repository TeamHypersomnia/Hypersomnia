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
			-- position here is only for debugging
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

function input_prediction_system:apply_correction(input_sequence, new_position, new_velocity) 
	-- we don't have more than one target at the moment
	for i=1, #self.targets do
		local prediction = self.targets[i].input_prediction
		
		print(input_sequence, #prediction.state_history)
		if input_sequence <= #prediction.state_history-1 then
			
			local correct_from = prediction.state_history[input_sequence]
			local simulation_entity = prediction.simulation_entity
			local simulation_body = simulation_entity.physics.body
			
			simulation_body:SetTransform(new_position, 0)
			simulation_body:SetLinearVelocity(new_velocity)
			
			local simulation_step = input_sequence
			
			local simulation_callback = function ()
				local state = prediction.state_history[simulation_step]
				local movement = simulation_entity.movement
				
				movement.moving_left = state.moving_left
				movement.moving_right = state.moving_right
				movement.moving_forward = state.moving_forward
				movement.moving_backward = state.moving_backward
			
				simulation_step = simulation_step + 1
			end
			
			self.simulation_world.substep_callbacks = {
				simulation_callback
			}
			
			self.simulation_world:process_steps(#prediction.state_history - input_sequence + 1)
			
			local corrected_pos = simulation_body:GetPosition()
			local corrected_vel = simulation_body:GetLinearVelocity()
			
			self.targets[i].cpp_entity.physics.body:SetTransform(corrected_pos, 0)
			self.targets[i].cpp_entity.physics.body:SetLinearVelocity(corrected_vel)
			
			local new_state_history = {}
			
			-- save states only more recent than input_sequence
			for j=input_sequence+1, #prediction.state_history do
				table.insert(new_state_history, prediction.state_history[j])
			end
			
			prediction.state_history = new_state_history
			
			clearlc(1)
			debuglc(1, rgba(255, 0, 0, 255), to_pixels(new_position), to_pixels(new_velocity) + to_pixels(new_velocity) )
			debuglc(1, rgba(0, 255, 0, 255), correct_from.position, correct_from.position+vec2(30, 0))
			debuglc(1, rgba(255, 255, 0, 255), to_pixels(corrected_pos), to_pixels(corrected_vel) + to_pixels(corrected_pos))
		end
	end
end

function input_prediction_system:update()
	for i=1, #self.targets do
		local prediction = self.targets[i].input_prediction
		--local movement_sync = self.targets[i].synchronization.modules.movement
		
		
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
	end
end

