melee_system = inherits_from (processing_system)

function melee_system:constructor(owner_world)
	self.owner_world = owner_world
	
	processing_system.constructor(self)
end

function melee_system:process_swinging()
	local msgs = self.owner_entity_system.messages["begin_swinging"]
	local client_sys = self.owner_entity_system.all_systems["client"]
				
	for i=1, #msgs do
		local msg = msgs[i]
		local target = msg.subject
		local weapon = target.weapon
		local entity = target.cpp_entity
		
		-- try to recover the owner entity if we're dealing with an item
		if target.item and target.item.wielder then
			entity = target.item.wielder.cpp_entity
		end
		
		local anim_msg = animate_message()
		
		if weapon.current_swing_direction then
			anim_msg.animation_type = animation_events.SWING_CW
			entity.movement.animation_message = animation_events.MOVE_CCW
		else
			anim_msg.animation_type = animation_events.SWING_CCW
			entity.movement.animation_message = animation_events.MOVE_CW
		end
		
		anim_msg.preserve_state_if_animation_changes = false
		anim_msg.change_animation = true
		anim_msg.change_speed = true
		anim_msg.speed_factor = 1
		anim_msg.subject = entity
		anim_msg.message_type = animate_message.START
		anim_msg.animation_priority = 10
		
		target.cpp_entity.owner_world:post_message(anim_msg)
		weapon.current_swing_direction = not weapon.current_swing_direction
		
		if weapon.transmit_bullets == true then
			client_sys.net_channel:post_reliable("SWING_REQUEST", {})
		end
	end
	
	msgs = self.owner_entity_system.messages["swing_hitcheck"]
	
	for i=1, #msgs do
		local msg = msgs[i]
		local target = msg.subject
		local weapon = target.weapon
		local entity = target.cpp_entity
				
		-- try to recover the owner entity if we're dealing with an item
		if target.item and target.item.wielder then
			entity = target.item.wielder.cpp_entity
		end
		
		local queried_area = vec2_vector()
		local num_verts = 8
		
		clearlc(2)
		
		for j=1, num_verts do
			local new_vert = entity.transform.current.pos + vec2.from_degrees(
				entity.transform.current.rotation - (weapon.swing_angle / 2) 
				+ (j-1) * (weapon.swing_angle / (num_verts - 1)) 
				) * weapon.swing_radius
		
			queried_area:add(new_vert)
			
			local prev = entity.transform.current.pos
			
			if j > 1 then
				prev = queried_area:at(j-1-1)
			end
			
			debuglc(2, rgba(255, 255, 255, 255), prev, new_vert)
		end
		
		debuglc(2, rgba(255, 255, 255, 255), queried_area:at(num_verts-1), entity.transform.current.pos)
		
		local bodies = self.owner_world.physics_system:query_polygon(queried_area, filters.BULLET, entity)
		
		while weapon.hits_remaining > 0 do
			for candidate in bodies.details do
				local hit_entity = body_to_entity(candidate.body)
				local hit_object = hit_entity.script
				
				if not weapon.entities_hit[hit_entity] then
					if hit_object then
						
					end
					
					burst_msg = particle_burst_message()
					burst_msg.subject = hit_entity
					burst_msg.pos = to_pixels(candidate.location)
					burst_msg.rotation = (entity.transform.current.pos -  to_pixels(candidate.location)):get_degrees()
					burst_msg.type = particle_burst_message.BULLET_IMPACT
					
					hit_entity.owner_world:post_message(burst_msg)
		
					weapon.entities_hit[hit_entity] = true
				end
			end
		
			weapon.hits_remaining = weapon.hits_remaining - 1
		end
	end
end