npc_class = { 
	entity = 0,
	weapon_animation_sets = {},
	current_weapon = bare_hands
}

function set_max_speed(entity, max_speed_val)
	entity.movement.max_speed = max_speed_val
	entity.steering.max_resultant_force = max_speed_val

end

function get_scripted(entity)
	return entity.scriptable.script_data
end

function init_npc(this_entity)
	for k, v in pairs(this_entity.scriptable.script_data) do print(k, v) end
	
	this_entity.scriptable.script_data = this_entity.scriptable.script_data:create()
	this_entity.scriptable.script_data.entity = this_entity
	this_entity.scriptable.script_data:init()
end

function npc_class:create(o)  
	 local inst = o or {}
     setmetatable(inst, { __index = npc_class } )
     return inst
end

function npc_class:init()
	local o = self
	
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
	
	o:take_weapon_item(shotgun)
end

function npc_class:refresh_behaviours() 
	self.entity.steering:clear_behaviours()
	
	for k, v in pairs(self.steering_behaviours) do
		self.entity.steering:add_behaviour(v)
	end
end

function npc_class:take_weapon_item(item_data)
		print("taking weapon...\n" .. item_data.animation_index)
		--print(item_data.weapon_info.is_melee)
		print(debug.traceback())
	recursive_write(self.entity.gun, item_data.weapon_info)
	
	for k, v in pairs(self.weapon_animation_sets) do print(k) end
	
	--for k, v in pairs(self.entity.gun.attributes) do
	--	print(k)
	--	print("\n")
	--end
	
	self.entity.animate.available_animations = self.weapon_animation_sets[item_data.animation_index]
	self.current_weapon = item_data
end

function npc_class:take_weapon(weapon_entity)
	take_weapon_item(get_scripted(weapon_entity).item_data)
	world:delete_entity(weapon_item)
end


function npc_class:drop_weapon()
	if self.current_weapon ~= bare_hands then
		print("dropping weapon...")
		local my_thrown_weapon = create_entity (archetyped(self.current_weapon.item_entity, {
			transform = {
				pos = self.entity.transform.current.pos
			}
		}))
		
		local throw_force = vec2.from_degrees(self.entity.transform.current.rotation) * 2
		
		local body = my_thrown_weapon.physics.body
		
		body:ApplyLinearImpulse(b2Vec2(throw_force.x, throw_force.y), body:GetWorldCenter())
		body:ApplyAngularImpulse(2)
		
		self:take_weapon_item(bare_hands)
	end
end

function npc_class:loop()
--	local entity = self.entity
--	local behaviours = self.steering_behaviours
--	local target_entities = self.target_entities
--	
--	
--	local myvel = entity.physics.body:GetLinearVelocity()
--	target_entities.forward.transform.current.pos = entity.transform.current.pos + vec2(myvel.x, myvel.y) * 50
--	
--	if entity.pathfinding:is_still_pathfinding() or entity.pathfinding:is_still_exploring() then
--		target_entities.navigation.transform.current.pos = entity.pathfinding:get_current_navigation_target()
--		
--		behaviours.obstacle_avoidance.enabled = true
--		if behaviours.sensor_avoidance.last_output_force:non_zero() then
--			behaviours.target_seeking.enabled = false
--			behaviours.forward_seeking.enabled = true
--			behaviours.obstacle_avoidance.enabled = true
--		else
--			behaviours.target_seeking.enabled = true
--			behaviours.forward_seeking.enabled = false
--			--behaviours.obstacle_avoidance.enabled = false
--		end
--	else
--		behaviours.target_seeking.enabled = false
--		behaviours.forward_seeking.enabled = false
--		--behaviours.obstacle_avoidance.enabled = false
--	end
--	
--	behaviours.sensor_avoidance.max_intervention_length = (entity.transform.current.pos - target_entities.navigation.transform.current.pos):length() - 70
--	
--	--	behaviours.sensor_avoidance.enabled = true
--	--	player_behaviours.obstacle_avoidance.enabled = true
--	--player_behaviours.forward_seeking.enabled = true
--	
--	if behaviours.obstacle_avoidance.last_output_force:non_zero() then
--		behaviours.wandering.current_wander_angle = behaviours.obstacle_avoidance.last_output_force:get_degrees()
--	end
	
	
	
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


npc_count = 1
my_npcs = {}

final_npc_archetype = (archetyped(character_archetype, {
		body = {
			transform = { pos = vec2(1000, (-2800)) },
			
			--behaviour_tree = {
			--	starting_node = npc_behaviour_tree.behave
			--},
		
			lookat = {
				target = "body",
				look_mode = lookat_component.VELOCITY,
				easing_mode = lookat_component.EXPONENTIAL,
				averages_per_sec = 30
			},
						
			scriptable = {
				available_scripts = npc_script_info,
				script_data = npc_class
			}
		},
		
		legs = {
			lookat = {
				target = "body",
				look_mode = lookat_component.VELOCITY,
				easing_mode = lookat_component.EXPONENTIAL,
				averages_per_sec = 10
			}
		}
}))

for i=1, npc_count do

	if i == 1 then 
		my_npcs[i] = create_entity_group(final_npc_archetype)
	else
		my_npcs[i] = archetyped(create_entity_group(final_npc_archetype), {
			body = {
				gun = assault_rifle.weapon_info
			}
		})
	end
	
	
	init_npc(my_npcs[i].body)
	get_scripted(my_npcs[i].body).
	weapon_animation_sets = {
		BARE_HANDS = npc_animation_body_set,
		FIREAXE = npc_animation_body_set,
		ASSAULT_RIFLE = npc_animation_body_shotgun_set,
		SHOTGUN = npc_animation_body_shotgun_set
	}	
	
	local script_data = get_scripted(my_npcs[i].body)
	script_data:refresh_behaviours()
	script_data:take_weapon_item(bare_hands)
	--my_npcs[i].body.pathfinding:start_exploring()
	
	my_npcs[i].body.gun.target_camera_to_shake:set(world_camera)
end