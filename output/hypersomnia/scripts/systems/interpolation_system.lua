interpolation_system = inherits_from (processing_system)

function interpolation_system:get_required_components()
	return { "interpolation", "cpp_entity" }
end

function interpolation_system:update()
	local msgs = self.owner_entity_system.messages["CURRENT_STEP"]
	
	local new_step = false
	local at_step_sequence;
	
	for i=1, #msgs do
		new_step = true
		at_step_sequence = msgs[i].data.at_step
	end
	
	--if new_step then
	for i=1, #self.targets do
		local target = self.targets[i]
		local new_position = self.targets[i].replication.modules.movement.position
		local new_velocity = self.targets[i].replication.modules.movement.velocity
		
		if new_position ~= nil then self.targets[i].cpp_entity.physics.body:SetTransform(new_position, 0) end
		if new_velocity ~= nil then self.targets[i].cpp_entity.physics.body:SetLinearVelocity(new_velocity) end
	end
	--end
end

