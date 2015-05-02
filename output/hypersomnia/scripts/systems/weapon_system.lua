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
		group.owner_world:post_message(destroy_message(group, nil))
	end
	
	processing_system.remove_entity(self, new_entity)
end


function weapon_system:shot_routine(target, premade_shot)
	-- remember about correct differentiation between requests that need to be processed
	-- for correctness and those on the client only for showing remote players' bullets

	local weapon = target.weapon
	local entity = target.cpp_entity
	
	-- try to recover the owner entity if we're dealing with an item
	if target.item ~= nil and target.item.wielder ~= nil then
		entity = target.item.wielder.cpp_entity
	end
	
	
	local gun_transform = transform_state(entity.transform.current)
	-- cancel out interpolation
	gun_transform.pos = to_pixels(entity.physics.body:GetPosition())
	
	if premade_shot ~= nil then
		gun_transform.pos = premade_shot.position
		gun_transform.rotation = premade_shot.rotation
	end
	
	local barrel_offset = vec2(weapon.bullet_barrel_offset):rotate(gun_transform.rotation, vec2())
	
	local barrel_transform = transform_state(gun_transform)
	barrel_transform.pos = barrel_transform.pos + barrel_offset
	
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
		["barrel_offset"] = barrel_offset,
		gun_transform = transform_state(gun_transform),
		barrel_transform = transform_state(barrel_transform),
		["premade_shot"] = premade_shot,
		bullets = {}
	}
	
	for i=1, weapon.bullets_once do
		local function unpack_bullet(random_generator)
			local vel = vec2.from_degrees(
				randval(
				gun_transform.rotation - weapon.spread_degrees,
				gun_transform.rotation + weapon.spread_degrees, random_generator))
	
			barrel_transform.rotation = vel:get_degrees()
				
			vel = vel * randval(weapon.bullet_speed, random_generator)
			
			return {
				pos = barrel_transform.pos,
				rotation = barrel_transform.rotation,
				["vel"] = vel
			}
		end
		
		table.insert(new_shot_message.bullets, unpack_bullet)
	end
	
	-- client - produce visual output from wherever the shot originates and inform the server about the shot
	-- server - broadcast the information about the shot and possibly store their ids for later resimulation
	self.owner_entity_system:post_table("shot_message", new_shot_message)
end

function components.weapon:can_be_unwielded()
	return next(self.buffered_actions) == nil
end

function weapon_system:handle_messages()
	local msgs = self.world_object:get_messages_filter_components("intent_message", { "wield" } )
	
	for i=1, #msgs do
		local item_subject = msgs[i].subject.script.wield.wielded_items[components.wield.keys.PRIMARY_WEAPON]
		
		if item_subject ~= nil and item_subject.weapon ~= nil then
			if msgs[i].intent == intent_message.SHOOT then
				if msgs[i].state_flag then
					if item_subject.weapon.is_melee or item_subject.weapon.current_rounds <= 0 then
						item_subject.weapon.trigger = components.weapon.triggers.MELEE
					else
						item_subject.weapon.trigger = components.weapon.triggers.SHOOT
					end
				else
					item_subject.weapon.trigger = components.weapon.triggers.NONE
				end
			end
		end
	end
end


function weapon_system:translate_shot_info_msgs()
	local msgs = self.owner_entity_system.messages["SHOT_INFO"]
	local object_by_id = self.owner_entity_system.all_systems["replication"].object_by_id
	
	for i=1, #msgs do
		local subject = object_by_id[msgs[i].data.subject_id].wield.wielded_items[components.wield.keys.PRIMARY_WEAPON]
		local forward_time = msgs[i].data.delay_time + self.owner_entity_system.all_systems["client"]:get_last_ping()/2
		
		table.insert(subject.weapon.buffered_actions, { trigger = components.weapon.triggers.SHOOT, premade_shot = {
			position = msgs[i].data.position,
			rotation = msgs[i].data.rotation,
			simulate_forward = forward_time,
			starting_global_id = msgs[i].data.starting_bullet_id,
			random_seed = msgs[i].data.random_seed
		}})
	end
	
	msgs = self.owner_entity_system.messages["SWING_INFO"]
	
	for i=1, #msgs do
		local subject = object_by_id[msgs[i].data.subject_id].wield.wielded_items[components.wield.keys.PRIMARY_WEAPON]
		
		table.insert(subject.weapon.buffered_actions, { trigger = components.weapon.triggers.MELEE, premade_shot = {}})
	end
end

function weapon_system:update()
	self:substep(self.delta_timer:extract_milliseconds())
end


function weapon_system:swinging_routine(target)
	-- the client checks for collisions with hit sensor
	
	
end

function weapon_system:substep(dt)
	--if dt <= 0 then return end
	
	for i=1, #self.targets do
		local target = self.targets[i]
		local weapon = target.weapon
		local state = weapon.state
		local states = components.weapon.states
		
		weapon.time_ms = weapon.time_ms + dt
		
		if state == states.READY then
			local trigger = weapon.trigger
			
			local premade_shot;
				
			if #weapon.buffered_actions > 0 then
				trigger = weapon.buffered_actions[1].trigger
				
				if weapon.buffered_actions[1].flag_msg then
					weapon.buffered_actions[1].flag_msg.handled = true
				end
				
				premade_shot = weapon.buffered_actions[1].premade_shot
				
				-- assume that a valid action will always be executed on "READY" state, 
				-- and right away pop it
				
				-- on the server: invalid action should anyway be invalidated
				-- on the client: the commands won't be constrained so they will be always executed
				table.remove(weapon.buffered_actions, 1)
			end
			
			local triggers = components.weapon.triggers
	
			if trigger == triggers.SHOOT then
				if weapon.constrain_requested_bullets then
					if weapon.current_rounds > 0 then
						self:shot_routine(target, premade_shot)
						
						if not weapon.is_automatic then
							weapon.trigger = triggers.NONE
						end
						
						weapon:set_state("SHOOTING_INTERVAL")
					end
				else
					self:shot_routine(target, premade_shot)
				end
			elseif trigger == triggers.MELEE then
				-- swinging is always automatic
				weapon:set_state("SWINGING")
				
				self.owner_entity_system:post_table("begin_swinging", {
					subject = target
				})
				
				weapon.hits_remaining = weapon.hits_per_swing
				weapon.entities_hit = {}
			end
		
		elseif state == states.SWINGING then
			if weapon:passed("swing_duration") then
				weapon:set_state("SWINGING_INTERVAL")
			else
				if weapon.hits_remaining > 0 then
					self.owner_entity_system:post_table("swing_hitcheck", {
						subject = target
					})
				end
			end
			
		elseif (state == states.SHOOTING_INTERVAL and weapon:passed("shooting_interval_ms")) or
			   (state == states.SWINGING_INTERVAL and weapon:passed("swing_interval_ms")) then
			weapon:set_state("READY")
		end
		
	end
end