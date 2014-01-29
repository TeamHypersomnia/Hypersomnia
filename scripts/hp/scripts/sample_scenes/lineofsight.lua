crosshair_sprite = create_sprite {
	image = images.crosshair,
	color = rgba(255, 0, 0, 255)
}

crosshair_blue = create_sprite {
	image = images.crosshair,
	color = rgba(255, 255, 255, 255)
}

blank_white = create_sprite {
	image = images.blank,
	color = rgba(255, 0, 0, 255)
}

blank_red = create_sprite {
	image = images.blank,
	size_multiplier = vec2(70, 10),
	color = rgba(255, 0, 0, 255)
}

blank_green = create_sprite {
	image = images.blank,
	size_multiplier = vec2(7, 7),
	color = rgba(0, 255, 0, 255)
}

blank_blue = create_sprite {
	image = images.blank,
	size_multiplier = vec2(17, 17),
	color = rgba(0, 0, 255, 255)
}

point_archetype = {
	image = images.metal,
	color = rgba(255, 255, 255, 255),
	texcoord = vec2(0, 0)
}

ground_point_archetype = {
	image = images.background,
	color = rgba(255, 255, 255, 255),
	texcoord = vec2(0, 0)
}


dofile "hp\\scripts\\sample_scenes\\map.lua"

small_box_archetype = {
	transform = {
		pos = vec2(0, 0), rotation = 0
	},
	
	render = {
		model = blank_red,
		layer = render_layers.OBJECTS
	},
	
	physics = {
		body_type = Box2D.b2_dynamicBody,
		
		body_info = {
			filter = filter_objects,
			shape_type = physics_info.RECT,
			rect_size = blank_red.size,
			
			linear_damping = 5,
			angular_damping = 5,
			fixed_rotation = false,
			density = 0.1,
			friction = 0,
			sensor = false
		}
	}
}

big_box_archetype = (archetyped(small_box_archetype, {
	render = {
		model = blank_blue
	},

	physics = {
		body_info = {
			rect_size = blank_blue.size,
			density = 0.5
		}
	}
}))

environment = create_entity {
	render = {
		model = environment_poly,
		layer = render_layers.OBJECTS
	},
	
	physics = {
		body_type = Box2D.b2_staticBody,
		
		body_info = {
			shape_type = physics_info.POLYGON,
			vertices = environment_poly,
			filter = filter_static_objects,
			friction = 0
		}
	},
	
	transform = {
		pos = vec2(-1000, 1000)
	}
}

ground = create_entity {
	render = {
		model = ground_poly,
		layer = render_layers.GROUND
	},
	
	transform = {
		pos = vec2(-1000, 1000)
	}
}


dofile "hp\\scripts\\sample_scenes\\steering.lua"
dofile "hp\\scripts\\sample_scenes\\npc.lua"
dofile "hp\\scripts\\sample_scenes\\behaviour_tree.lua"

npc_script_info = create_scriptable_info {
	scripted_events = {
		[scriptable_component.LOOP] 	=
			function(message)
				get_scripted(message):loop()
			end
	}
}

my_npc_archetype = {
	body = {
		transform = { 
			pos = vec2(0, 0),
			rotation = 0
		},
		
		render = {
			layer = render_layers.PLAYERS,
			model = blank_green
		},
		
		physics = {
			body_type = Box2D.b2_dynamicBody,
			
			body_info = {
				filter = filter_characters,
				shape_type = physics_info.RECT,
				rect_size = blank_green.size,
				
				angular_damping = 5,
				linear_damping = 18,
				
				fixed_rotation = true,
				density = 0.1
			},
		},
		
		visibility = {
			visibility_layers = {
				[visibility_component.DYNAMIC_PATHFINDING] = {
					square_side = 5000,
					color = rgba(0, 255, 255, 120),
					ignore_discontinuities_shorter_than = -1,
					filter = filter_pathfinding_visibility
				}
				--,
				--
				--[visibility_component.CONTAINMENT] = {
				--	square_side = 250,
				--	color = rgba(0, 255, 255, 120),
				--	ignore_discontinuities_shorter_than = -1,
				--	filter = filter_obstacle_visibility
				--}
			}
		},
		
		pathfinding = {
			enable_backtracking = true,
			target_offset = 100,
			rotate_navpoints = 10,
			distance_navpoint_hit = 2,
			favor_velocity_parallellness = true
		},
		
		movement = {
			input_acceleration = vec2(70000, 70000),
			max_speed = 4300
		},
		
		steering = {
			max_resultant_force = -1 -- -1 = no force clamping
		},
		
		scriptable = {
			available_scripts = npc_script_info,
			script_data = npc_class
		}
	}
}

player = create_entity_group (archetyped(my_npc_archetype, {
	body = {
		transform = {},
		
		input = {
			intent_message.MOVE_FORWARD,
			intent_message.MOVE_BACKWARD,
			intent_message.MOVE_LEFT,
			intent_message.MOVE_RIGHT
		}
	},

	crosshair = { 
		transform = {
			pos = vec2(0, 0),
			rotation = 0
		},
		
		render = {
			layer = render_layers.GUI_OBJECTS,
			model = crosshair_sprite
		},
		
		crosshair = {
			sensitivity = 5.5
		},
		
		chase = {
			target = "body",
			relative = true
		},
		
		input = {
			intent_message.AIM
		}
	}
}))

set_max_speed(player.body, 5000)

init_scripted(player.body)

npc_count = 1
my_npcs = {}

for i=1, npc_count do
	my_npcs[i] = create_entity_group (archetyped(my_npc_archetype, {
		body = {
			transform = { pos = vec2(1000, (-2800)) }
			,
			
			behaviour_tree = {
				starting_node = npc_behaviour_tree.behave
			}
		}
	}))
	
	init_scripted(my_npcs[i].body)
	
	get_scripted(my_npcs[i].body):refresh_behaviours()
	--my_npcs[i].body.pathfinding:start_exploring()
end


main_context = create_input_context {
	intents = { 
		[mouse.raw_motion] 		= intent_message.AIM,
		[keys.W] 				= intent_message.MOVE_FORWARD,
		[keys.ESC] 				= custom_intents.QUIT,
		[keys.S] 				= intent_message.MOVE_BACKWARD,
		[keys.A] 				= intent_message.MOVE_LEFT,
		[keys.D] 				= intent_message.MOVE_RIGHT,
		[keys.R] 				= custom_intents.STEERING_REQUEST,
		[keys.E] 				= custom_intents.EXPLORING_REQUEST,
		[keys.V] 				= custom_intents.INSTANT_SLOWDOWN,
		[mouse.rdown] 			= intent_message.SWITCH_LOOK,
		[mouse.rdoubleclick] 	= intent_message.SWITCH_LOOK,
		[mouse.wheel]			= custom_intents.ZOOM_CAMERA,
		[keys.ADD] 				= custom_intents.SPEED_INCREASE,
		[keys.SUBTRACT] 		= custom_intents.SPEED_DECREASE
	}
}

input_system:add_context(main_context)

camera_archetype = {
	transform = {
		pos = vec2(),
		rotation = 0
	},

	camera = {
		enabled = true,
		
		layer = 0, -- 0 = topmost
		mask = render_component.WORLD,
		
		enable_smoothing = true,
		smoothing_average_factor = 0.5,
		averages_per_sec = 20,
		
		crosshair = nil,
		player = nil,
	
		orbit_mode = camera_component.LOOK,
		max_look_expand = vec2(config_table.resolution_w/2, config_table.resolution_h/2)*10,
		angled_look_length = 100
	},
	
	chase = {
		relative = false,
		offset = vec2(config_table.resolution_w/(-2), config_table.resolution_h/(-2))
	}
}


current_zoom_level = 3000

function set_zoom_level(camera)
	local mult = 1 + (current_zoom_level / 1000)
	local new_w = config_table.resolution_w*mult
	local new_h = config_table.resolution_h*mult
	camera.camera.ortho = rect_ltrb(rect_xywh((config_table.resolution_w-new_w)/2, (config_table.resolution_h-new_h)/2, new_w, new_h))
	
	player.crosshair.crosshair.size_multiplier = vec2(mult, mult)
	target_entity.crosshair.size_multiplier = vec2(mult, mult)
end

scriptable_zoom = create_scriptable_info {
	scripted_events = {
		[scriptable_component.INTENT_MESSAGE] = function(message)
				if message.intent == custom_intents.ZOOM_CAMERA then
					current_zoom_level = current_zoom_level-message.wheel_amount
					set_zoom_level(message.subject)
				end
			return true
		end
	}
}

world_camera = create_entity (archetyped(camera_archetype, {
	transform = {
		pos = vec2(),
		rotation = 0
	},

	camera = {
		screen_rect = rect_xywh(0, 0, config_table.resolution_w, config_table.resolution_h),
		ortho = rect_ltrb(0, 0, config_table.resolution_w, config_table.resolution_h),
	
		crosshair = player.crosshair,
		player = player.body
	},
	
	input = {
		intent_message.SWITCH_LOOK,
		custom_intents.ZOOM_CAMERA
	},
	
	chase = {
		target = player.body
	},
	
	scriptable = {
		available_scripts = scriptable_zoom
	}
}))


steer_request_fnc = 
			function()
				target_entity.transform.current.pos = player.crosshair.transform.current.pos
				
				get_scripted(player.body):refresh_behaviours(player.body)
			end
			
loop_only_info = create_scriptable_info {
	scripted_events = {	
		[scriptable_component.INTENT_MESSAGE] = 
			function(message)
				if message.intent == custom_intents.QUIT then
					input_system.quit_flag = 1	
				elseif message.intent == custom_intents.STEERING_REQUEST then
					steer_request_fnc()
					player.body.pathfinding:start_pathfinding(target_entity.transform.current.pos)
				elseif message.intent == custom_intents.EXPLORING_REQUEST then
					steer_request_fnc()
					player.body.pathfinding:start_exploring()
				elseif message.intent == custom_intents.INSTANT_SLOWDOWN then
					physics_system.timestep_multiplier = 0.000
				elseif message.intent == custom_intents.SPEED_INCREASE then
					physics_system.timestep_multiplier = physics_system.timestep_multiplier + 0.05
				elseif message.intent == custom_intents.SPEED_DECREASE then
					physics_system.timestep_multiplier = physics_system.timestep_multiplier - 0.05
					
					if physics_system.timestep_multiplier < 0.01 then
						physics_system.timestep_multiplier = 0.01
					end
				end
				return true
			end
	}
}

myloopscript = create_entity {
	input = {
			custom_intents.STEERING_REQUEST,
			custom_intents.EXPLORING_REQUEST,
			custom_intents.SPEED_INCREASE,
			custom_intents.SPEED_DECREASE,
			custom_intents.INSTANT_SLOWDOWN,
			custom_intents.QUIT
	},
		
	scriptable = {
		available_scripts = loop_only_info,
		
		script_data = { 
		 jemcoupe = {
			twujstary = "eloguwno2"
		}
		}
	}
}

set_zoom_level(world_camera)