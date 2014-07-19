lifetime_system = inherits_from (processing_system)

function lifetime_system:constructor(world_object)
	self.world_object = world_object
	processing_system.constructor(self)
end


function lifetime_system:get_required_components()
	return { "lifetime" }
end

function lifetime_system:update()
	local to_delete = {}

	local msgs = self.world_object:get_messages_filter_components("collision_message", { "lifetime" } )
	
	--print "here"
	
	for i=1, #msgs do
		local message = msgs[i]
		
		print "Hit!"
		burst_msg = particle_burst_message()
		burst_msg.subject = message.collider
		burst_msg.pos = message.point
		burst_msg.rotation = (message.subject_impact_velocity * -1):get_degrees()
		burst_msg.type = particle_burst_message.BULLET_IMPACT
		
		message.collider.owner_world:post_message(burst_msg)
		
		table.insert(to_delete, message.subject.script)
	end

	for i=1, #self.targets do
		local lifetime = self.targets[i].lifetime
		
		if	(lifetime.max_lifetime_ms > -1 
			and lifetime.current_lifetime:get_milliseconds() >= lifetime.max_lifetime_ms )
		or
			(lifetime.max_distance > -1
			and (self.targets[i].cpp_entity.transform.current.pos - lifetime.starting_point):length() >= lifetime.max_distance)
		then
			table.insert(to_delete, self.targets[i])
		end
	end
	
	for i=1, #to_delete do
		self.owner_entity_system:remove_entity(to_delete[i])
	end
end