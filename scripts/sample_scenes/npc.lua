npc_class = { 
	entity = 0,
	weapon_animation_sets = {
		
	}
}

function set_max_speed(entity, max_speed_val)
	entity.movement.max_speed = max_speed_val
	entity.steering.max_resultant_force = max_speed_val

end

function get_scripted(entity)
	return entity.scriptable.script_data
end

function init_scripted(this_entity) 
	this_entity.scriptable.script_data = this_entity.scriptable.script_data:new { entity = this_entity }
end

function npc_class:new(o)
	o = o or {}
	o.steering_behaviours = {
		target_seeking = behaviour_state(target_seek_steering),
		forward_seeking = behaviour_state(forward_seek_steering),
		
		sensor_avoidance = behaviour_state(sensor_avoidance_steering),
		wandering = behaviour_state(wander_steering),
		obstacle_avoidance = behaviour_state(obstacle_avoidance_steering),
		separating = behaviour_state(separation_steering),
		pursuit = behaviour_state(pursuit_steering)
	}
	
	o.target_entities = {
		navigation = create_entity(target_entity_archetype),
		forward = create_entity(target_entity_archetype),
		last_seen = create_entity(target_entity_archetype)
	}
	
	o.was_seen = false
		
	o.steering_behaviours.forward_seeking.target_from:set(o.target_entities.forward)
	o.steering_behaviours.target_seeking.target_from:set(o.target_entities.navigation)
	o.steering_behaviours.sensor_avoidance.target_from:set(o.target_entities.navigation)
	
	o.steering_behaviours.pursuit.enabled = false
	o.steering_behaviours.pursuit.target_from:set(player.body)
		
    setmetatable(o, self)
    self.__index = self
    return o
end

function npc_class:refresh_behaviours() 
	self.entity.steering:clear_behaviours()
	
	for k, v in pairs(self.steering_behaviours) do
		self.entity.steering:add_behaviour(v)
	end
end

function npc_class:take_weapon_item(item_data)
	recursive_write(self.entity.gun, item_data.weapon_info)
	self.entity.animate = weapon_animation_sets[item_data.animation_index]
end

function npc_class:take_weapon(weapon_entity)
	take_weapon_item(get_scripted(weapon_entity).item_data)
	world:delete_entity(weapon_item)
end

function npc_class:drop_weapon()
	create_entity {
		
		
	}
end

function npc_class:loop()
	local entity = self.entity
	local behaviours = self.steering_behaviours
	local target_entities = self.target_entities
	
	my_atlas:_bind()
	
	local myvel = entity.physics.body:GetLinearVelocity()
	target_entities.forward.transform.current.pos = entity.transform.current.pos + vec2(myvel.x, myvel.y) * 50
	
	if entity.pathfinding:is_still_pathfinding() or entity.pathfinding:is_still_exploring() then
		target_entities.navigation.transform.current.pos = entity.pathfinding:get_current_navigation_target()
		
		behaviours.obstacle_avoidance.enabled = true
		if behaviours.sensor_avoidance.last_output_force:non_zero() then
			behaviours.target_seeking.enabled = false
			behaviours.forward_seeking.enabled = true
			behaviours.obstacle_avoidance.enabled = true
		else
			behaviours.target_seeking.enabled = true
			behaviours.forward_seeking.enabled = false
			--behaviours.obstacle_avoidance.enabled = false
		end
	else
		behaviours.target_seeking.enabled = false
		behaviours.forward_seeking.enabled = false
		--behaviours.obstacle_avoidance.enabled = false
	end
	
	behaviours.sensor_avoidance.max_intervention_length = (entity.transform.current.pos - target_entities.navigation.transform.current.pos):length() - 70
	
	--	behaviours.sensor_avoidance.enabled = true
	--	player_behaviours.obstacle_avoidance.enabled = true
	--player_behaviours.forward_seeking.enabled = true
	
	if behaviours.obstacle_avoidance.last_output_force:non_zero() then
		behaviours.wandering.current_wander_angle = behaviours.obstacle_avoidance.last_output_force:get_degrees()
	end
	
	if entity ~= player.body then 
	
	end
--	
--	local p1 = entity.transform.current.pos
--	local p2 = player.body.transform.current.pos
--	
--	render_system:push_line(debug_line(p1, p2, rgba(255, 0, 0, 255)))
--	
--	--print(p1.x, p1.y)
--	--print(p2.x, p2.y)
--	--if p1.x == p2.x and p1.y == p2.y then return true end
--	
--	ray_output = physics_system:ray_cast(p1, p2, create(b2Filter, filter_obstacle_visibility), entity)
--	
--	if ray_output.hit and ray_output.what_entity == player.body then
--	
--		render_system:push_line(debug_line(p1, ray_output.intersection, rgba(0, 255, 0, 255)))
--		
--		behaviours.pursuit.enabled = true
--		
--		self.was_seen = true
--		self.target_entities.last_seen.transform.current.pos = player.body.transform.current.pos
--		self.target_entities.navigation.transform.current.pos = player.body.transform.current.pos
--		
--		entity.pathfinding:clear_pathfinding_info()
--	else
--		behaviours.pursuit.enabled = false
--		
--		if self.was_seen and not entity.pathfinding:is_still_pathfinding() then
--			entity.pathfinding:start_pathfinding(self.target_entities.last_seen.transform.current.pos)
--			
--			entity.pathfinding:start_exploring()
--		end
--	end
	
	return true
end