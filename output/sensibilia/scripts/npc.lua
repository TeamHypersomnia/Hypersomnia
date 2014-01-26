npc_class = inherits_from {}

function npc_class:initialize(subject_entity) 
	self.jump_timer = timer()
	self.entity = subject_entity
	self.foot_sensor_p1 = vec2(0, 0)
	self.foot_sensor_p2 = vec2(0, 0)
end
	
function npc_class:jump() 
	if self.jump_timer:get_milliseconds() > 100 then
		
		local pos = self.entity.transform.current.pos
		
		local query_rect_p1 = pos + self.foot_sensor_p1
		local query_rect_p2 = pos + self.foot_sensor_p2
		local query_rect = vec2_vector()
		
		add_vals(query_rect, { 
			query_rect_p1,
			vec2(query_rect_p2.x, query_rect_p1.y),
			query_rect_p2,
			vec2(query_rect_p1.x, query_rect_p2.y)
		})
		
		local jump_off_candidates = physics_system:query_polygon(query_rect, create(b2Filter, filter_npc_feet), self.entity)
		
		local can_jump = false
		print "\nCollisions:\n"
		for candidate in jump_off_candidates.bodies do
			print (body_to_entity(candidate).name)
			can_jump = true
		end
		
		if can_jump then
			local target_body = self.entity.physics.body
			target_body:ApplyLinearImpulse(b2Vec2(0, -100), target_body:GetWorldCenter(), true)
			print("CAN JUMP!!!!")
		end
		
		self.jump_timer:reset()
	end
end

function npc_class:set_foot_sensor_from_sprite(subject_sprite, thickness) 
	self.foot_sensor_p1 = vec2(-subject_sprite.size.x / 2, subject_sprite.size.y / 2)
	self.foot_sensor_p2 = vec2( subject_sprite.size.x / 2, subject_sprite.size.y / 2 + thickness) 
end

npc_group_archetype = {
	body = {
		physics = {
			body_type = Box2D.b2_dynamicBody,
			
			body_info = {
				shape_type = physics_info.RECT,
				radius = 60,
				--rect_size = vec2(30, 30),
				filter = filter_objects,
				density = 1,
				friction = 1,
				
				--,
				fixed_rotation = true
			}	
		},
		
		render = {
			layer = render_layers.OBJECTS
		},
		
		transform = {},
		
		movement = {
			input_acceleration = vec2(10000, 10000),
			max_speed = 1000,
			max_speed_animation = 2300,
			
			receivers = {},
			
			force_offset = vec2(0, 5)
			
			--receivers = {
			--	{ target = "body", stop_at_zero_movement = false }, 
			--	{ target = "legs", stop_at_zero_movement = true  }
			--}
		},
		
		scriptable = {}
	}
}

function spawn_npc(group_overrider)
	local my_new_npc = create_entity_group (archetyped(npc_group_archetype, group_overrider))
	
	local new_npc_scriptable = npc_class:create()
	new_npc_scriptable:initialize(my_new_npc.body)
	
	my_new_npc.body.scriptable.script_data = new_npc_scriptable
	
	return my_new_npc
end