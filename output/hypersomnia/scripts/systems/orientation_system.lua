orientation_system = inherits_from (processing_system)

function orientation_system:get_required_components()
	return { "orientation", "cpp_entity" }
end

function orientation_system:update()
	for i=1, #self.targets do
		local target = self.targets[i]
		local orientation = target.orientation
		
		if orientation.receiver and target.replication.modules.crosshair.position ~= nil then
			orientation.crosshair_entity.transform.current.pos = target.replication.modules.crosshair.position + target.cpp_entity.transform.current.pos
		else--if orientation:cmd_rate_ready() then
			--orientation:cmd_rate_reset()
			local current_pos =  vec2(orientation.crosshair_entity.transform.current.pos - target.cpp_entity.transform.current.pos)
			
			if orientation.last_pos == nil or (orientation.last_pos - current_pos):length() > 0 then
				orientation.last_pos = vec2(current_pos)
				self.owner_entity_system.all_systems["client"].net_channel:reliable_seq_msg("CROSSHAIR_SNAPSHOT", { position = current_pos }, true)
			end
		end
	end
end