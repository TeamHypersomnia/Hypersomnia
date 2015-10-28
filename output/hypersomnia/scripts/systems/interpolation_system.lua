interpolation_system = inherits_from (processing_system)

function interpolation_system:constructor(simulation_world) 
	self.simulation_world = simulation_world
	
	processing_system.constructor(self)
end

function interpolation_system:get_required_components()
	return { "interpolation" }
end

function interpolation_system:add_entity(new_entity)
	local interpolation = new_entity.interpolation
	
	if interpolation.extrapolate then
		interpolation.simulation_entity = interpolation.create_simulation_entity()
	end
	
	processing_system.add_entity(self, new_entity)
end

function interpolation_system:remove_entity(removed_entity)
	local sim_entity = removed_entity.interpolation.simulation_entity
	
	if sim_entity then
		local owner_world = sim_entity.owner_world
		owner_world:post_message(destroy_message(sim_entity, entity_id()))
	end

	processing_system.remove_entity(self, removed_entity)
end


function interpolation_system:update()
	local msgs = self.owner_entity_system.messages["CURRENT_STEP"]
	
	local steps_forward = math.floor( 0.5 + (self.owner_entity_system.all_systems["client"]:get_last_ping()) / (1000/config_table.tickrate))
	
	local new_step = false
	local at_step_sequence;
	
	for i=1, #msgs do
		new_step = true
		at_step_sequence = msgs[i].data.at_step
	end
	
	--if new_step then
	for i=1, #self.targets do
		local target = self.targets[i]
		
		if target.cpp_entity.physics then
			local movement_module = self.targets[i].replication.modules.movement
			if movement_module == nil then
				movement_module = self.targets[i].replication.modules.movement_rotated
			end
			
			if movement_module ~= nil and (
				movement_module.FIELDS_READ.position or
				movement_module.FIELDS_READ.velocity or
				movement_module.FIELDS_READ.angle or
				movement_module.FIELDS_READ.angular_velocity
				)
			then
				movement_module.FIELDS_READ.position = nil
				movement_module.FIELDS_READ.velocity = nil
				movement_module.FIELDS_READ.angle = nil
				movement_module.FIELDS_READ.angular_velocity = nil
				
				local body = self.targets[i].cpp_entity.physics.body
				
				local new_position = movement_module.position
				local new_velocity = movement_module.velocity
				local new_angle = movement_module.angle
				local new_angular_velocity = movement_module.angular_velocity
				
				if new_angle == nil then
					new_angle = 0
				end
				
				local target_body = body
				
				local interpolation = target.interpolation
				
				if interpolation.extrapolate and steps_forward > 0 then
					target_body = interpolation.simulation_entity.physics.body
				end
				
				if new_position ~= nil then target_body:SetTransform(new_position, new_angle) end
				if new_velocity ~= nil then target_body:SetLinearVelocity(new_velocity) end
				if new_angular_velocity ~= nil then target_body:SetAngularVelocity(new_angular_velocity) end
				
				if interpolation.extrapolate and steps_forward > 0 then
					--print "extrapolating.."
					--print (steps_forward)
					local last_results = {}
					local step_number = 1
					
					self.simulation_world.prestep_callbacks = {}
					self.simulation_world.poststep_callbacks = {
						function()
							if step_number > steps_forward - 4 then
								last_results[#last_results + 1] = {
									pos = b2Vec2(target_body:GetPosition()),
									angle = target_body:GetAngle() + 0,
									angular_vel = target_body:GetAngularVelocity() + 0,
									vel = b2Vec2(target_body:GetLinearVelocity())
								}
							end
							
							step_number = step_number + 1
						end
					}
					
					-- jitter may make us several steps ahead or behind, so simulate 2 more just in case
					self.simulation_world:process_steps(steps_forward + 2)
					self.simulation_world.poststep_callbacks = {}
					
					-- find the result of the smallest discrepancy
					
					local actual_pos = to_pixels(target.cpp_entity.physics.body:GetPosition())
					
					local smallest_dist;
					local best_candidate;
					
					for r=1, #last_results do
						local new_dist = (to_pixels(last_results[r].pos) - actual_pos):length_sq()
						
						if smallest_dist == nil or new_dist < smallest_dist then
							smallest_dist = new_dist
							best_candidate = r
						end
					end
					
					local corrected = last_results[best_candidate]
					
					if math.sqrt(smallest_dist) > config_table.divergence_radius then
						print(math.sqrt(smallest_dist))
						target.cpp_entity.physics.body:SetTransform(corrected.pos, corrected.angle)
						target.cpp_entity.physics.body:SetLinearVelocity(corrected.vel)
						target.cpp_entity.physics.body:SetAngularVelocity(corrected.angular_vel)
					end
				end
			end
		end
	end
end

