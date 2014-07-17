input_prediction_system = inherits_from (processing_system)

function input_prediction_system:constructor(simulation_world) 
	self.simulation_world = simulation_world
	
	processing_system.constructor(self)
end

function input_prediction_system:get_required_components()
	return { "input_prediction", "cpp_entity" }
end

function input_prediction_system:substep()
	for i=1, #self.targets do
		local prediction = self.targets[i].input_prediction
		local movement = self.targets[i].cpp_entity.movement 
		
		local history_entry = { 
			-- position here is only for debugging
			position = b2Vec2(self.targets[i].cpp_entity.physics.body:GetPosition()),
			moving_left = movement.moving_left,
			moving_right = movement.moving_right,
			moving_forward = movement.moving_forward,
			moving_backward = movement.moving_backward
		}
		clearl()
		
		prediction.state_history[prediction.first_state + prediction.count] = history_entry
		prediction.count = prediction.count + 1
		
		if prediction.count > 100 then
			prediction.state_history[prediction.first_state] = nil
			prediction.first_state = prediction.first_state + 1
			prediction.count = prediction.count - 1
		end
		
		for j=prediction.first_state, prediction.first_state+prediction.count-1 do
			debuglb2(rgba(255, 255, 255, 255), prediction.state_history[j].position)
		end
		
		
		if prediction.count <= 1 or not table.compare(history_entry, prediction.state_history[prediction.first_state + prediction.count - 2], { position = true } ) 
								 --or not table.compare(history_entry, prediction.state_history[prediction.first_state + prediction.count - 3], { position = true } )
		then
			prediction.same_since = prediction.first_state+prediction.count-1
			--self.owner_entity_system.all_systems["client"].net_channel:post_reliable_bs(protocol.write_msg("INPUT_SNAPSHOT", to_send))
		end
		
		--if prediction.last_acked_step-10 <= prediction.same_since then
			local to_send = {}
			rewrite(to_send, history_entry, { position = true } )
			
			to_send.at_step = prediction.first_state + prediction.count - 1
			
			--global_logfile:write("\nStep: " .. to_send.at_step)
			--global_logfile:write("\nleft: " .. movement.moving_left)
			--global_logfile:write("\nright: " .. movement.moving_right)
			--global_logfile:write("\nforward: " .. movement.moving_forward)
			--global_logfile:write("\nbackward: " .. movement.moving_backward)
			--print ("sending snapshot " .. to_send.at_step)
			self.owner_entity_system.all_systems["client"].substep_unreliable:WriteBitstream(protocol.write_msg("INPUT_SNAPSHOT", to_send))
		--end
		
	end
end

function input_prediction_system:update()
	local msgs = self.owner_entity_system.messages["CURRENT_STEP"]
	
	local new_step = false
	local recent_data;
	
	for i=1, #msgs do
		new_step = true
		recent_data = msgs[i].data
	end
	
	if new_step then
		for i=1, #self.targets do
			local at_step_sequence = recent_data.at_step+1
			local target = self.targets[i]
			local new_position = self.targets[i].synchronization.modules.movement.position
			local new_velocity = self.targets[i].synchronization.modules.movement.velocity
			-- we don't have more than one target at the moment
			
			local prediction = target.input_prediction
			prediction.last_acked_step = at_step_sequence
			--print ("recvd snapshot " .. at_step_sequence)
			
			if prediction.count > 0 and at_step_sequence < prediction.first_state + prediction.count and at_step_sequence >= prediction.first_state then
				local correct_from = prediction.state_history[at_step_sequence]
				
				--print((to_pixels(new_position) - to_pixels(correct_from.position)):length())
				
				local simulation_entity = prediction.simulation_entity
				local simulation_body = simulation_entity.physics.body
				
				simulation_body:SetTransform(new_position, 0)
				simulation_body:SetLinearVelocity(new_velocity)
				
				local simulation_step = at_step_sequence
				
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
				
				self.simulation_world:process_steps(prediction.first_state + prediction.count - at_step_sequence)
				
				local corrected_pos = simulation_body:GetPosition()
				local corrected_vel = simulation_body:GetLinearVelocity()
				
				if (to_pixels(corrected_pos) - to_pixels(target.cpp_entity.physics.body:GetPosition())):length() > config_table.divergence_radius then
					print((to_pixels(corrected_pos) - to_pixels(target.cpp_entity.physics.body:GetPosition())):length())
					target.cpp_entity.physics.body:SetTransform(corrected_pos, 0)
					target.cpp_entity.physics.body:SetLinearVelocity(corrected_vel)
				end
				
				local new_state_history = {}
				
				-- save states only more recent than at_step_sequence
				local cnt = 0
				for j=at_step_sequence+1, prediction.first_state+prediction.count-1 do
					new_state_history[j] = prediction.state_history[j]
					cnt=cnt+1
				end
				
				prediction.first_state = at_step_sequence+1
				prediction.count = cnt
				
				prediction.state_history = new_state_history
				
				clearlc(1)
				debuglc(1, rgba(255, 0, 0, 255), to_pixels(new_position), to_pixels(new_position) + to_pixels(new_velocity) )
				--debuglc(1, rgba(0, 255, 0, 255), to_pixels(correct_from.position), to_pixels(correct_from.position) + to_pixels(correct_from.vel))
				debuglc(1, rgba(0, 255, 255, 255), to_pixels(corrected_pos), (to_pixels(corrected_vel) + to_pixels(corrected_pos)))
			end
		end
	end
end

