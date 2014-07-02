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
			-- position here is only for debugging
			position = b2Vec2(self.targets[i].cpp_entity.physics.body:GetPosition()),
			vel = b2Vec2(self.targets[i].cpp_entity.physics.body:GetLinearVelocity()),
			moving_left = movement.moving_left,
			moving_right = movement.moving_right,
			moving_forward = movement.moving_forward,
			moving_backward = movement.moving_backward
		}
		clearl()
		
		prediction.state_history[prediction.first_state + prediction.count] = history_entry
		prediction.count = prediction.count + 1
		
		if prediction.count > 60 then
			prediction.state_history[prediction.first_state] = nil
			prediction.first_state = prediction.first_state + 1
			prediction.count = prediction.count - 1
		end
		
		for j=prediction.first_state, prediction.first_state+prediction.count-1 do
			debuglb2(rgba(255, 255, 255, 255), prediction.state_history[j].position)
		end
		
		local command_bs = BitStream()
		
		command_bs:WriteByte(protocol.messages.INPUT_SNAPSHOT)
		command_bs:WriteUint(prediction.first_state + prediction.count - 1)
		command_bs:WriteBit(movement.moving_left > 0)
		command_bs:WriteBit(movement.moving_right > 0)
		command_bs:WriteBit(movement.moving_forward > 0)
		command_bs:WriteBit(movement.moving_backward > 0)
		
		self.owner_entity_system.all_systems["client"].net_channel:post_bitstream(command_bs)
		self.owner_entity_system.all_systems["client"].cmd_requested = true
		
		
		
		global_logfile:write(("Step number.." .. (prediction.first_state + prediction.count - 1)))
		global_logfile:write(("\nApplying inputs.."))
		global_logfile:write(("\nLeft:" .. history_entry.moving_left))
		global_logfile:write(("\nRight:" .. history_entry.moving_right))
		global_logfile:write(("\nForward:" .. history_entry.moving_forward))
		global_logfile:write(("\nBackward:" .. history_entry.moving_backward))
		global_logfile:write("\n")
	end
end

function input_prediction_system:apply_correction(input_sequence, new_position, new_velocity) 
	-- we don't have more than one target at the moment
	for i=1, #self.targets do
		local prediction = self.targets[i].input_prediction
		
		--input_sequence = prediction.first_state
		if prediction.count > 0 and input_sequence < prediction.first_state + prediction.count and input_sequence >= prediction.first_state then
			local correct_from = prediction.state_history[input_sequence]
			
			local simulation_entity = prediction.simulation_entity
			local simulation_body = simulation_entity.physics.body
			
			--print(correct_from.position.x, correct_from.position.y)
			--print(to_pixels(new_position).x, to_pixels(new_position).y)
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
			
			self.simulation_world:process_steps(prediction.first_state + prediction.count - input_sequence)
			
			local corrected_pos = simulation_body:GetPosition()
			local corrected_vel = simulation_body:GetLinearVelocity()
			
			local vel = to_pixels(self.targets[i].cpp_entity.physics.body:GetLinearVelocity())
			print((to_pixels(corrected_pos) - to_pixels(self.targets[i].cpp_entity.physics.body:GetPosition())):length())
			
			if (to_pixels(corrected_pos) - to_pixels(self.targets[i].cpp_entity.physics.body:GetPosition())):length() > config_table.divergence_radius then
				self.targets[i].cpp_entity.physics.body:SetTransform(corrected_pos, 0)
				self.targets[i].cpp_entity.physics.body:SetLinearVelocity(corrected_vel)
			end
			
			local new_state_history = {}
			
			-- save states only more recent than input_sequence
			local cnt = 0
			for j=input_sequence+1, prediction.first_state+prediction.count-1 do
				new_state_history[j] = prediction.state_history[j]
				cnt=cnt+1
			end
			
			--prediction.first_state = input_sequence+1
			--prediction.count = cnt
			
			--prediction.state_history = new_state_history
			
			--if (to_pixels(corrected_pos) - to_pixels(self.targets[i].cpp_entity.physics.body:GetPosition())):length() > 0.0 then
			--end
			
			--print(corrected_pos.x, corrected_pos.y)
			--print(corrected_vel.x, corrected_vel.y)
			clearlc(1)
			debuglc(1, rgba(255, 0, 0, 255), to_pixels(new_position), to_pixels(new_position) + to_pixels(new_velocity) )
			--debuglc(1, rgba(0, 255, 0, 255), to_pixels(correct_from.position), to_pixels(correct_from.position) + to_pixels(correct_from.vel))
			debuglc(1, rgba(0, 255, 255, 255), to_pixels(corrected_pos), (to_pixels(corrected_vel) + to_pixels(corrected_pos)))
		end
	end
end

function input_prediction_system:update()
	for i=1, #self.targets do
		local prediction = self.targets[i].input_prediction
		--local movement_sync = self.targets[i].synchronization.modules.movement
		
		
		-- write the most recent prediction and request correction
		--local prediction_request = BitStream()
		--prediction_request:name_property("CLIENT_PREDICTION")
		--prediction_request:WriteByte(protocol.messages.CLIENT_PREDICTION)
		--prediction_request:name_property("input_sequence")
		-- we're already ahead of one prediction
		-- as substep callback is called before b2World::Step
		--prediction_request:WriteUint(prediction.first_state + prediction.count - 1)
		-- write our current position
		
		--self.owner_entity_system.all_systems["client"].net_channel.unreliable_buf:WriteBitstream(prediction_request)
	end
end

