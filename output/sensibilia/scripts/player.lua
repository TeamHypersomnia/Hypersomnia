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


player_scriptable_info = create_scriptable_info {
	scripted_events = {
		[scriptable_component.INTENT_MESSANT] = function (message) 
			if message.intent == custom_intents.JUMP then
				
				
				
				
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
			intent_message.MOVE_LEFT,
			intent_message.MOVE_RIGHT
		}
	}
}