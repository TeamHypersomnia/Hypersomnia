player_sprite = create_sprite {
	image = images.blank,
	size = vec2(30, 100)
}

debug_sensor = create_sprite {
	image = images.blank,
	size = vec2(40, 5),
	color = rgba(0, 0, 255, 122)
}

player_debug_circle = simple_create_polygon (reversed(gen_circle_vertices(60, 5)))
map_uv_square(player_debug_circle, images.blank)


function get_self(entity)
	return entity.scriptable.script_data
end

player_scriptable_info = create_scriptable_info {
	scripted_events = {
		[scriptable_component.INTENT_MESSANT] = function (message) 
			if message.intent == custom_intents.JUMP then
				local this = get_self(message.subject)
					if this.jump_timer:get_milliseconds() > 100 then
					
					local jump_off_candidates = physics_system:query_aabb(this.foot_sensor_p1, this.foot_sensor_p2, filter_npc_feet, message.subject)
					local can_jump = false
					
					for jump_off_candidates.bodies do
						can_jump = true
					end
					
					if can_jump then
						local target_body = message.subject.physics.body
						target_body:ApplyLinearImpulse(b2Vec2(0, 3), target_body:GetWorldCenter()) 
					end
				end
			end
		end
	}
}

player = create_entity_group {
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
			model = player_sprite,
			layer = render_layers.OBJECTS
		},
		
		transform = {
			pos = vec2(100, -50)
		},
		
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
		
		input = {
			--intent_message.MOVE_FORWARD,
			--intent_message.MOVE_BACKWARD,
			custom_intents.JUMP
			intent_message.MOVE_LEFT,
			intent_message.MOVE_RIGHT
		}
	}
}