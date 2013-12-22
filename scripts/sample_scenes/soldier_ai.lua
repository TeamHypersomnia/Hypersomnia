dofile "scripts\\sample_scenes\\camera.lua"

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

dofile "scripts\\sample_scenes\\map.lua"

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
	
	particle_emitter = {
		available_particle_effects = metal_effects
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

bullet_sprite = create_sprite {
	image = images.bullet,
	size_multiplier = vec2(1.0, 0.6)
}

dofile "scripts\\sample_scenes\\steering.lua"
dofile "scripts\\sample_scenes\\weapons.lua"
dofile "scripts\\sample_scenes\\soldier_tree.lua"

character_archetype = {
	body = {
		transform = { 
			pos = vec2(0, 0),
			rotation = 0
		},
		
		render = {
			layer = render_layers.PLAYERS,
			model = blank_green
		},
		
		animate = {},
		gun = {},
		
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
		      
		particle_emitter = {
			available_particle_effects = npc_effects
		},
		
		pathfinding = {
			enable_backtracking = true,
			target_offset = 100,
			rotate_navpoints = 10,
			distance_navpoint_hit = 2,
			favor_velocity_parallellness = true
		},
		
		movement = {
			input_acceleration = vec2(30000, 30000),
			max_speed = 4300,
			max_speed_animation = 1200,
			
			receivers = {
				{ target = "body", stop_at_zero_movement = false }, 
				{ target = "legs", stop_at_zero_movement = true  }
			}
		},
		
		steering = {
			max_resultant_force = -1 -- -1 = no force clamping
		},
		 
		children = {
			"legs"
		}
	},
	
	legs = {
		transform = { 
			pos = vec2(0, 0),
			rotation = 0
		},
		
		render = {
			layer = render_layers.LEGS,
			model = nil
		},
		
		chase = {
			target = "body"
		},
		
		lookat = {
			target = "body",
			look_mode = lookat_component.VELOCITY
		},
		
		animate = {
			available_animations = npc_animation_legs_set
		}
	}
}

dofile "scripts\\sample_scenes\\npc.lua"
dofile "scripts\\sample_scenes\\player.lua"

loop_only_info = create_scriptable_info {
	scripted_events = {	
		[scriptable_component.INTENT_MESSAGE] = 
			function(message)
				if message.intent == custom_intents.QUIT then
					input_system.quit_flag = 1	
				elseif message.intent == custom_intents.DROP_WEAPON then
					if message.state_flag then
						get_scripted(player.body):drop_weapon()
					end
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
			end,
			
		[scriptable_component.LOOP] = function(subject)
			my_atlas:_bind()
			
			for k, v in pairs(my_npcs) do get_scripted(v.body):loop() end
		end
	}
}

myloopscript = create_entity {
	input = {
			custom_intents.SPEED_INCREASE,
			custom_intents.SPEED_DECREASE,
			custom_intents.INSTANT_SLOWDOWN,
			custom_intents.QUIT,
			custom_intents.DROP_WEAPON
	},
		
	scriptable = {
		available_scripts = loop_only_info,
		script_data = {}
	}
}
