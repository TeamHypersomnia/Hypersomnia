main_context = create_input_context {
	intents = { 
		[mouse.raw_motion] 		= intent_message.AIM,
		[keys.ESC] 				= custom_intents.QUIT,
		[keys.W] 				= intent_message.MOVE_FORWARD,
		[keys.S] 				= intent_message.MOVE_BACKWARD,
		[keys.A] 				= intent_message.MOVE_LEFT,
		[keys.D] 				= intent_message.MOVE_RIGHT,
		[keys.R] 				= custom_intents.RESTART,
		[keys.V] 				= custom_intents.INSTANT_SLOWDOWN,
		
		[mouse.ldoubleclick] 	= intent_message.SHOOT,
		[mouse.ltripleclick] 	= intent_message.SHOOT,
		[mouse.ldown] 			= intent_message.SHOOT,
		
		[keys.LSHIFT] 			= intent_message.SWITCH_LOOK,
		[mouse.rdown] 			= custom_intents.DROP_WEAPON,
		[mouse.rdoubleclick] 	= custom_intents.DROP_WEAPON,
		[mouse.wheel]			= custom_intents.ZOOM_CAMERA,
		[keys.ADD] 				= custom_intents.SPEED_INCREASE,
		[keys.SUBTRACT] 		= custom_intents.SPEED_DECREASE
	}
}

input_system:clear_contexts()
input_system:add_context(main_context)


dofile "sensibilia\\scripts\\camera.lua"
current_zoom_level = 2000
set_zoom_level(world_camera)

function set_color(poly, col)
	for i = 0, poly:get_vertex_count()-1 do
		poly:get_vertex(i).color = col
	end
end

environment_archetype = {
	physics = {
		body_type = Box2D.b2_staticBody,
		
		body_info = {
			shape_type = physics_info.POLYGON,
			filter = filter_static_objects,
			density = 1,
			friction = 100
		}
	},
	
	render = {
		layer = render_layers.BACKGROUND
	},
	
	transform = {
	
	}
}

ground_poly = simple_create_polygon (reversed {
	vec2(0, 500) + vec2(-800, 0),
	vec2(0, 500) + vec2(500, 0),
	vec2(0, 500) + vec2(900, -200),
	vec2(0, 500) + vec2(1400, -300),
	vec2(0, 500) + vec2(3000, -300),
	vec2(0, 500) + vec2(3000, 200),
	vec2(0, 500) + vec2(-800, 200)
})

map_uv_square(ground_poly, images.blank)
set_color(ground_poly, rgba(0, 255, 0, 255))

player_sprite = create_sprite {
	image = images.blank,
	size = vec2(30, 100)
}

player_debug_circle = simple_create_polygon (reversed(gen_circle_vertices(60, 5)))
map_uv_square(player_debug_circle, images.blank)

environment_entity = create_entity (archetyped(environment_archetype, {
	render = {
		model = ground_poly
	}
}))


player = create_entity_group {
	body = {
		physics = {
			body_type = Box2D.b2_dynamicBody,
			
			body_info = {
				shape_type = physics_info.CIRCLE,
				radius = 60,
				filter = filter_objects,
				density = 1,
				friction = 100
				--,
				--fixed_rotation = true
			}	
		},
		
		render = {
			model = player_debug_circle,
			layer = render_layers.OBJECTS
		},
		
		transform = {
			pos = vec2(100, -50)
		},
		
		movement = {
			input_acceleration = vec2(30000, 300000),
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
			intent_message.MOVE_FORWARD,
			intent_message.MOVE_BACKWARD,
			intent_message.MOVE_LEFT,
			intent_message.MOVE_RIGHT
		}
	}
}


loop_only_info = create_scriptable_info {
	scripted_events = {
		[scriptable_component.INTENT_MESSAGE] = 
			function(message)
				if message.intent == custom_intents.QUIT then
					input_system.quit_flag = 1
				elseif message.intent == custom_intents.RESTART then
						set_world_reloading_script(reloader_script)
				elseif message.intent == custom_intents.INSTANT_SLOWDOWN then
					physics_system.timestep_multiplier = 0.00001
				elseif message.intent == custom_intents.SPEED_INCREASE then
					physics_system.timestep_multiplier = physics_system.timestep_multiplier + 0.05
				elseif message.intent == custom_intents.SPEED_DECREASE then
					physics_system.timestep_multiplier = physics_system.timestep_multiplier - 0.05
					
					if physics_system.timestep_multiplier < 0.01 then
						physics_system.timestep_multiplier = 0.01
					end
				end
				
				return false
			end,
				
		[scriptable_component.LOOP] = function(subject)
			my_atlas:_bind()
		end
	}
}


create_entity {
	input = {
			custom_intents.SPEED_INCREASE,
			custom_intents.SPEED_DECREASE,
			custom_intents.INSTANT_SLOWDOWN,
			custom_intents.QUIT,
			custom_intents.RESTART
	},
		
	scriptable = {
		available_scripts = loop_only_info
	}	
}

physics_system.b2world:SetGravity(b2Vec2(0, 100))