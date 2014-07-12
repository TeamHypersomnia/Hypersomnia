orientation_system = inherits_from (processing_system)

function orientation_system:get_required_components()
	return { "orientation", "cpp_entity" }
end

function orientation_system:update()
	for i=1, #self.targets do
		local target = self.targets[i]
		local orientation = target.orientation
		
		if orientation.receiver then
			orientation.crosshair_entity.transform.current.pos = target.synchronization.modules.crosshair.position + target.cpp_entity.transform.current.pos
		else--if orientation:cmd_rate_ready() then
			--orientation:cmd_rate_reset()
			local current_pos =  vec2(orientation.crosshair_entity.transform.current.pos - target.cpp_entity.transform.current.pos)
			
			local send_retry;
			
			if orientation.last_pos == nil or (orientation.last_pos - current_pos):length() > 2 then
				orientation.idle = false
				orientation.idle_timer:reset()
				
				orientation.last_pos = vec2(current_pos)
				send_retry = false
			end
			
			if orientation.idle_timer:get_milliseconds() > 100 and not orientation.idle then
				orientation.idle = true
				send_retry = true
			end
			
			if send_retry ~= nil then 
				self.owner_entity_system.all_systems["client"].net_channel:reliable_seq_msg("CROSSHAIR_SNAPSHOT", { position = current_pos }, send_retry)
			end
			
		end
	end
end