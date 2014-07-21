weapon_system = inherits_from (processing_system)

function weapon_system:constructor(world_object, physics)
	self.world_object = world_object
	self.physics = physics
	self.delta_timer = timer()
	processing_system.constructor(self)
end

function weapon_system:get_required_components()
	return { "weapon" }
end

function weapon_system:remove_entity(new_entity)
	local group = new_entity.weapon.barrel_smoke_group 
	
	if group ~= nil then
		group.owner_world:delete_entity(group, nil)
	end
end


function weapon_system:shot_routine(target, premade_shot)
	-- remember about correct differentiation between requests that need to be processed
	-- for correctness and those on the client only for showing remote players' bullets

	local weapon = target.weapon
	local entity = target.cpp_entity
	
	local gun_transform = transform_state(entity.transform.current)
	-- cancel out interpolation
	gun_transform.pos = to_pixels(entity.physics.body:GetPosition())
		
	if premade_shot ~= nil then
		gun_transform.pos = premade_shot.position
		gun_transform.rotation = premade_shot.rotation
	end
	
	local barrel_transform = transform_state(gun_transform)
	barrel_transform.pos = barrel_transform.pos + vec2(weapon.bullet_barrel_offset):rotate(gun_transform.rotation, vec2())
	
	-- this chunk won't be executed only for remote players on the client
	if weapon.constrain_requested_bullets then
		weapon.current_rounds = weapon.current_rounds - 1
		-- buffer a security-check for the distance between the entity position and the requested position
		-- it will be executed somewhere else
		
		local result = self.physics:ray_cast(gun_transform.pos, barrel_transform.pos, weapon.bullet_entity.physics.body_info.filter, entity)
	
		if result.hit then
			barrel_transform.pos = result.intersection
		end
	end
	
	local new_shot_message = {
		subject = target,
		gun_transform = transform_state(gun_transform),
		barrel_transform = transform_state(barrel_transform),
		["premade_shot"] = premade_shot,
		bullets = {}
	}
		
	for i=1, weapon.bullets_once do
		local vel = vec2.from_degrees(
			randval(
			gun_transform.rotation - weapon.spread_degrees,
			gun_transform.rotation + weapon.spread_degrees))

		barrel_transform.rotation = vel:get_degrees()
			
		vel = vel * randval(weapon.bullet_speed)

		table.insert(new_shot_message.bullets, {
			id = weapon.next_bullet_id,
			pos = barrel_transform.pos,
			rotation = barrel_transform.rotation,
			["vel"] = vel
		})	
		
		weapon.next_bullet_id = weapon.next_bullet_id + 1
	end
	
	-- client - produce visual output from wherever the shot originates and inform the server about the shot
	-- server - broadcast the information about the shot and possibly store their ids for later resimulation
	self.owner_entity_system:post_table("shot_message", new_shot_message)
end

function weapon_system:handle_messages()
	local msgs = self.world_object:get_messages_filter_components("intent_message", { "weapon" } )
	
	for i=1, #msgs do
		if msgs[i].intent == intent_message.SHOOT then
			if msgs[i].state_flag then
				msgs[i].subject.script.weapon.trigger = components.weapon.triggers.SHOOT
			else
				msgs[i].subject.script.weapon.trigger = components.weapon.triggers.NONE
			end
		end
	end
end


function weapon_system:translate_shot_info_msgs()
	local msgs = self.owner_entity_system.messages["SHOT_INFO"]
	
	for i=1, #msgs do
	--print "getting info"
	--print(msgs[i].data.subject_id)
	--print(self.owner_entity_system.all_systems["synchronization"].my_sync_id)
		local subject = self.owner_entity_system.all_systems["synchronization"].object_by_id[msgs[i].data.subject_id]
		local forward_time = msgs[i].data.delay_time + self.owner_entity_system.all_systems["client"]:get_last_ping()/2
		--print(forward_time)
		--print(subject.weapon.transmit_bullets)
		subject.weapon.next_bullet_id = msgs[i].data.starting_bullet_id
		
		table.insert(subject.weapon.buffered_actions, { trigger = components.weapon.triggers.SHOOT, premade_shot = {
			position = msgs[i].data.position,
			rotation = msgs[i].data.rotation,
			simulate_forward = forward_time
		}})
	end
end

function weapon_system:update()
	self:substep(self.delta_timer:extract_milliseconds())
end

function weapon_system:substep(dt)
	if dt <= 0 then return end
	
	for i=1, #self.targets do
		local target = self.targets[i]
		local weapon = target.weapon
		local state = weapon.state
		local states = components.weapon.states
		
		weapon.time_ms = weapon.time_ms + dt
		
		if state == states.READY then
			local trigger = weapon.trigger
			local entity = target.cpp_entity
			
			local premade_shot;
				
			if #weapon.buffered_actions > 0 then
				trigger = weapon.buffered_actions[1].trigger
				premade_shot = weapon.buffered_actions[1].premade_shot
				
				-- assume that a valid action will always be executed on "READY" state, 
				-- and right away pop it
				
				-- on the server: invalid action should anyway be invalidated
				-- on the client: the commands won't be constrained so they will be always executed
				table.remove(weapon.buffered_actions, 1)
			end
			
			local triggers = components.weapon.triggers
	
			if trigger == triggers.MELEE then
				--weapon:set_state("SWINGING")
				--begin_swinging_routine()
			elseif trigger == triggers.SHOOT then
				if weapon.constrain_requested_bullets then
					if weapon.current_rounds > 0 then
						self:shot_routine(target, premade_shot)
						
						if not weapon.is_automatic then
							trigger = triggers.NONE
						end
						
						weapon:set_state("SHOOTING_INTERVAL")
					end
				else
					self:shot_routine(target, premade_shot)
				end
			end
		
		elseif state == states.SWINGING then
			if weapon:passed("swing_duration") then
				weapon:set_state("SWINGING_INTERVAL")
			else
				swinging_routine()
			end
			
		elseif (state == states.SHOOTING_INTERVAL and weapon:passed("shooting_interval_ms")) or
			   (state == states.SWINGING_INTERVAL and weapon:passed("swing_interval_ms")) then
			weapon:set_state("READY")
		end
		
	end
end