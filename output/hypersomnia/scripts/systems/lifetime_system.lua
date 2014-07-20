lifetime_system = inherits_from (processing_system)

function lifetime_system:constructor(world_object)
	self.world_object = world_object
	processing_system.constructor(self)
end


function lifetime_system:remove_entity(removed_entity)
	local id = removed_entity.lifetime.bullet_id
	
	if id ~= nil then
		removed_entity.lifetime.sender.weapon.existing_bullets[id] = nil
	end

	processing_system.remove_entity(self, removed_entity)
end

function lifetime_system:get_required_components()
	return { "lifetime" }
end

function lifetime_system:translate_hit_infos()
	local msgs = {}
	local objects = self.owner_entity_system.all_systems["synchronization"].object_by_id

	local hit_infos = self.owner_entity_system.messages["HIT_INFO"]

	for i=1, #hit_infos do
		local msg = hit_infos[i]
		print "received hit info"
		print (msg.data.sender_id)
		print (msg.data.victim_id)
		print (msg.data.bullet_id)
		
		local victim = objects[msg.data.victim_id]
		local sender = objects[msg.data.sender_id]
		local bullet = objects[msg.data.sender_id].weapon.existing_bullets[msg.data.bullet_id]
		
		local new_collision = {}
		
		if bullet ~= nil then
			new_collision.subject = bullet.owner_entity.cpp_entity
			new_collision.subject_impact_velocity = to_pixels(bullet.owner_entity.cpp_entity.physics.body:GetLinearVelocity())
		else
			new_collision.subject_impact_velocity = vec2(1, 0)
		end
	
		new_collision.collider = victim.cpp_entity
		new_collision.point = vec2(victim.cpp_entity.transform.current.pos)
		new_collision.sender = sender
	    
		table.insert(msgs, new_collision)
	end
	
	self:resolve_collisions(msgs)
end

function lifetime_system:poststep()
	self:resolve_collisions(self.world_object:get_messages_filter_components("collision_message", { "lifetime" } ))
end

function lifetime_system:resolve_collisions(msgs)
	local client_sys = self.owner_entity_system.all_systems["client"]
	local needs_send = false
	
	for i=1, #msgs do
		local message = msgs[i]
		
		-- we could have posted info about a remote bullet that already does not exist
		if message.subject ~= nil then
			local collider_script = message.collider.script
			local lifetime = message.subject.script.lifetime
		
			if lifetime.sender.weapon.transmit_bullets 
				and collider_script ~= nil and collider_script.synchronization ~= nil
			then
				needs_send = true
				
				client_sys.net_channel:post_reliable("HIT_REQUEST", {
					victim_id = collider_script.synchronization.id,
					bullet_id = lifetime.bullet_id
				})
			end
	
			self.owner_entity_system:post_remove(message.subject.script)
		end
		
		burst_msg = particle_burst_message()
		burst_msg.subject = message.collider
		burst_msg.pos = message.point
		burst_msg.rotation = (message.subject_impact_velocity * -1):get_degrees()
		burst_msg.type = particle_burst_message.BULLET_IMPACT
		
		message.collider.owner_world:post_message(burst_msg)
	end
	
	if needs_send then
		client_sys:send_all_data()
	end
end

function lifetime_system:update()
	for i=1, #self.targets do
		local lifetime = self.targets[i].lifetime
		
		if	(lifetime.max_lifetime_ms > -1 
			and lifetime.current_lifetime:get_milliseconds() >= lifetime.max_lifetime_ms )
		or
			(lifetime.max_distance > -1
			and (self.targets[i].cpp_entity.transform.current.pos - lifetime.starting_point):length() >= lifetime.max_distance)
		then
			self.owner_entity_system:post_remove(self.targets[i])
		end
	end
end