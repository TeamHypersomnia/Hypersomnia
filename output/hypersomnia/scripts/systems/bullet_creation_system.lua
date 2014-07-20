bullet_creation_system = inherits_from (processing_system)

function bullet_creation_system:constructor(world_object, camera_to_shake)
	self.camera_to_shake = camera_to_shake
	self.simulation_world = simulation_world
	
	self.world_object = world_object
	self.world = world_object.world
	
	processing_system.constructor(self)
end

function bullet_creation_system:update()
	local msgs = self.owner_entity_system.messages["shot_message"]
	
	for i=1, #msgs do
		local target = msgs[i].subject
		local weapon = target.weapon
		local entity = target.cpp_entity
	
		local msg = animate_message()
		msg.animation_type = animation_events.SHOT
		msg.preserve_state_if_animation_changes = false
		msg.change_animation = true
		msg.change_speed = true
		msg.speed_factor = 1
		msg.subject = entity
		msg.message_type = animate_message.START
		msg.animation_priority = 1
		
		self.world:post_message(msg)
		
		local burst = particle_burst_message()
		burst.pos = msgs[i].barrel_transform.pos
		burst.rotation = msgs[i].barrel_transform.rotation
		burst.subject = entity
		burst.type = particle_burst_message.WEAPON_SHOT
		
		if weapon.barrel_smoke_group ~= nil then
			burst.target_group_to_refresh:set(weapon.barrel_smoke_group)
		end
		
		self.world:post_message(burst)
		
		local client_sys = self.owner_entity_system.all_systems["client"]
		
		for b=1, #msgs[i].bullets do
			local bullet = msgs[i].bullets[b]
			
			if self.camera_to_shake ~= nil then 
				local shake_dir = vec2()
				
				shake_dir:set_from_degrees(randval(
					bullet.rotation - weapon.shake_spread_degrees,
					bullet.rotation + weapon.shake_spread_degrees))
		
				self.camera_to_shake.camera.last_interpolant.pos = self.camera_to_shake.camera.last_interpolant.pos + shake_dir * weapon.shake_radius
			end
			
			local premade_shot = msgs[i].premade_shot
			if premade_shot ~= nil and premade_shot.simulate_forward ~= nil then
				local v1 = msgs[i].gun_transform.pos
				local v2 = bullet.pos + (bullet.vel*premade_shot.simulate_forward/1000)
				local result = self.world_object.physics_system
							:ray_cast(v1, v2, weapon.bullet_entity.physics.body_info.filter, entity)
				
				if result.hit then
					bullet.pos = result.intersection
				else
					bullet.pos = bullet.pos + (bullet.vel*premade_shot.simulate_forward/1000)
				end
			end
			
			local bullet_entity = self.world_object:create_entity (override(weapon.bullet_entity, { 
				transform = { 
					pos = bullet.pos,
					rotation = bullet.rotation
				}
			}))
			
			self.owner_entity_system:add_entity(components.create_components {
				lifetime = {
					max_lifetime_ms = weapon.max_lifetime_ms
					--max_distance = weapon.max_bullet_distance,
					--starting_point = vec2(bullet.pos)
				},
				
				cpp_entity = bullet_entity
			})
			
			
			local body = bullet_entity.physics.body
			body:SetLinearVelocity(to_meters(bullet.vel))
		end
		
		
		if target.weapon.transmit_bullets == true then
			if #msgs[i].bullets == 1 then
				client_sys.net_channel:post_reliable("SHOT_REQUEST", {
					position = msgs[i].gun_transform.pos,
					rotation = msgs[i].gun_transform.rotation
				})
			elseif #msgs[i].bullets > 1 then
			
			end
			
			-- bullet events are so important that they need to be issued right away
			client_sys:send_all_data()
		end
	end
end