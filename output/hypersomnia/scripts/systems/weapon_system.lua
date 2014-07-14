weapon_system = inherits_from (processing_system)

function weapon_system:constructor(world_object)
	self.world_object = world_object
	self.physics = world_object.physics_system
	self.delta_timer = timer()
	processing_system.constructor(self)
end

function weapon_system:get_required_components()
	return { "weapon" }
end

function weapon_system:shot_routine(target)
	local weapon = target.weapon
	local entity = target.cpp_entity
	
	local gun_transform = entity.transform.current
	local gun_rotation = gun_transform.rotation
	
	weapon.current_rounds = weapon.current_rounds - 1
	
	local new_transform = transform_state(gun_transform)
	new_transform.pos = new_transform.pos + vec2(weapon.bullet_distance_offset):rotate(gun_rotation, vec2())

	local result = self.physics:ray_cast(gun_transform.pos, new_transform.pos, create(b2Filter, weapon.bullet_entity.physics.body_info.filter), entity);

	if result.hit then
		new_transform.pos = result.intersection
	end
	
	local new_shot_message = {
		subject = target,
		bullets = {}
	}
		
	for i=1, weapon.bullets_once do
		local vel = vec2.from_degrees(
			randval(
			gun_transform.rotation - weapon.spread_degrees,
			gun_transform.rotation + weapon.spread_degrees))

		new_transform.rotation = vel:get_degrees()
			
		vel = vel * randval(weapon.bullet_speed)
		
		table.insert(new_shot_message.bullets, { 
			pos = new_transform.pos,
			rotation = new_transform.rotation,
			["vel"] = vel
		})	
	end
	
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
			local trigger = weapon.trigger;
			local triggers = components.weapon.triggers
	
			if trigger == triggers.MELEE then
				--weapon:set_state("SWINGING")
				--begin_swinging_routine()
			elseif trigger == triggers.SHOOT then
				if weapon.current_rounds > 0 then
					self:shot_routine(target)
					
					if not weapon.is_automatic then
						trigger = triggers.NONE
					end
	
					weapon:set_state("SHOOTING_INTERVAL")
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