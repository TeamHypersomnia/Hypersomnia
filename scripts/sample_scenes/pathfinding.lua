physics_system.timestep_multiplier = 1

visibility_system.draw_cast_rays = 0
visibility_system.draw_triangle_edges = 0
visibility_system.draw_discontinuities = 1
visibility_system.draw_visible_walls = 0


visibility_system.epsilon_ray_angle_variation = 0.0001
visibility_system.epsilon_threshold_obstacle_hit = 10
visibility_system.epsilon_distance_vertex_hit = 2

pathfinding_system.draw_memorised_walls = 1
pathfinding_system.draw_undiscovered = 1
pathfinding_system.epsilon_max_segment_difference = 4
pathfinding_system.epsilon_distance_visible_point = 2
pathfinding_system.epsilon_distance_the_same_vertex = 50

render_system.draw_steering_forces = 1
render_system.draw_substeering_forces = 1
render_system.draw_velocities = 0

render_system.draw_avoidance_info = 1
render_system.draw_wandering_info = 1

render_system.visibility_expansion = 1.0
render_system.max_visibility_expansion_distance = 1
render_system.draw_visibility = 0

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
	size_multiplier = vec2(10, 10),
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

function map_uv_square(texcoords_to_map, lefttop, bottomright)
	for k, v in pairs(texcoords_to_map) do
		v.texcoord = vec2(
		(v.pos.x - lefttop.x) / (bottomright.x-lefttop.x),
		(v.pos.y - lefttop.y) / (bottomright.y-lefttop.y)
		)
	end
end

point_archetype = {
	image = images.blank,
	color = rgba(0, 30, 30, 255),
	texcoord = vec2(0, 0)
}

size_mult = vec2(80, -80)*3.33

map_points = {
	square = {
		U = archetyped(point_archetype, { pos = vec2(-5.1, 6.29) * size_mult }),
		V = archetyped(point_archetype, { pos = vec2(-5.16, -4.68) * size_mult, image = images.blank, texcoord = vec2(0, 1), color = rgba(0, 150, 150, 255) }),
		W = archetyped(point_archetype, { pos = vec2(8.19, -4.39) * size_mult, image = images.blank, texcoord = vec2(1, 1), color = rgba(0, 150, 150, 255) }),
		Z = archetyped(point_archetype, { pos = vec2(8.19, 6.87) * size_mult, image = images.blank, texcoord = vec2(1, 0), color = rgba(0, 150, 150, 255) })
	},
	
	interior = {
		A = archetyped(point_archetype, { pos = vec2(-3, -2) 		 * size_mult }),
		B = archetyped(point_archetype, { pos = vec2(-3.13, -3.49)  * size_mult }),
		C = archetyped(point_archetype, { pos = vec2(7.06, -3.33)  	* size_mult }),
		D = archetyped(point_archetype, { pos = vec2(7.1, 5.74)  	* size_mult }),
		E = archetyped(point_archetype, { pos = vec2(-4.32, 5.71)  * size_mult }),
		F = archetyped(point_archetype, { pos = vec2(-4.22, 3.52)  * size_mult }),
		G = archetyped(point_archetype, { pos = vec2(-3.2, 3.52) 	 * size_mult }),
		H = archetyped(point_archetype, { pos = vec2(-3.23, 4.81)  * size_mult }),
		I = archetyped(point_archetype, { pos = vec2(-2.55, 4.78)  * size_mult }),
		J = archetyped(point_archetype, { pos = vec2(-2.96, -1.4)  * size_mult }),
		K = archetyped(point_archetype, { pos = vec2(3.5, -1.62)  * size_mult }),
		L = archetyped(point_archetype, { pos = vec2(3.44, 1.76)  * size_mult }),
		M = archetyped(point_archetype, { pos = vec2(3.78, 1.72)  * size_mult }),
		N = archetyped(point_archetype, { pos = vec2(3.85, 0)  		* size_mult }),
		O = archetyped(point_archetype, { pos = vec2(5.01, 0)  		* size_mult }),
		P = archetyped(point_archetype, { pos = vec2(4.97, 3.84)  * size_mult }),
		Q = archetyped(point_archetype, { pos = vec2(3.36, 3.54)  * size_mult }),
		R = archetyped(point_archetype, { pos = vec2(3.43, 4.29)  * size_mult }),
		S = archetyped(point_archetype, { pos = vec2(5.46, 4.52)  * size_mult }),
		T = archetyped(point_archetype, { pos = vec2(5.49, -1.94)  * size_mult })
	}
}

map_uv_square(map_points.interior, map_points.square.U.pos, map_points.square.W.pos)

environment_poly = create_polygon
{
	map_points.square.V,
	map_points.interior.B,
	map_points.interior.A,
	map_points.interior.T,
	map_points.interior.S,
	map_points.interior.R,
	map_points.interior.Q,
	map_points.interior.P,
	map_points.interior.O,
	map_points.interior.N,
	map_points.interior.M,
	map_points.interior.L,
	map_points.interior.K,
	map_points.interior.J,
	map_points.interior.I,
	map_points.interior.H,
	map_points.interior.G,
	map_points.interior.F,
	map_points.interior.E,
	map_points.interior.D,
	map_points.interior.C,
	map_points.interior.B,
	map_points.square.V,
	map_points.square.W,
	map_points.square.Z,
	map_points.square.U
}

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
		model = environment_poly
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
		pos = vec2(0, 300)
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
				linear_damping = 3,
				
				fixed_rotation = false,
				density = 0.1
			},
		},
		
		movement = {
			input_acceleration = vec2(30000, 30000),
			
			air_resistance = 0.0,
			max_speed = 3000
		},
	}
}

create_entity (archetyped(small_box_archetype, {
		transform = {
			pos = vec2(100, -600),
			rotation = 0
		}
}))

create_entity (archetyped(small_box_archetype, {
		transform = {
			pos = vec2(400, -432),
			rotation = 20
		}
}))

create_entity (archetyped(small_box_archetype, {
		transform = {
			pos = vec2(800, -432),
			rotation = -30
		}
}))

create_entity (archetyped(big_box_archetype, {
		transform = {
			pos = vec2(200+(-170), 340),
			rotation = 0
		}
}))

create_entity (archetyped(big_box_archetype, {
		transform = {
			pos = vec2(200+(-500), 300),
			rotation = 0
		}
}))
	
create_entity (archetyped(big_box_archetype, {
	transform = {
		pos = vec2((-280), 500),
		rotation = 0
	}
}))


player = create_entity_group (archetyped(my_npc_archetype, {
	body = {
		transform = {},
		
		physics = {
		
			body_info = {
				fixed_rotation = true,
				angular_damping = 5,
				linear_damping = 13
			}
		},
		
		input = {
			intent_message.MOVE_FORWARD,
			intent_message.MOVE_BACKWARD,
			intent_message.MOVE_LEFT,
			intent_message.MOVE_RIGHT
		},
	
		visibility = {
			visibility_layers = {
				[visibility_component.CONTAINMENT] = {
					square_side = 7000,
					ignore_discontinuities_shorter_than = 80,
					color = rgba(255, 0, 255, 0),
					filter = filter_obstacle_visibility
				},	
				
				[visibility_component.DYNAMIC_PATHFINDING] = {
					square_side = 7000,
					color = rgba(0, 255, 255, 120),
					ignore_discontinuities_shorter_than = 80,
					filter = filter_pathfinding_visibility
				}
			}
		},
		
		pathfinding = {
			enable_backtracking = true,
			target_offset = 100,
			distance_navpoint_hit = 2,
			starting_ignore_discontinuities_shorter_than = 150
		},
		
		movement = {
			max_speed = 4300
		},
		
		steering = {
			max_resultant_force = 4300 -- -1 = no force clamping
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

main_context = create_input_context {
	intents = { 
		[mouse.raw_motion] 		= intent_message.AIM,
		[keys.W] 				= intent_message.MOVE_FORWARD,
		[keys.S] 				= intent_message.MOVE_BACKWARD,
		[keys.A] 				= intent_message.MOVE_LEFT,
		[keys.D] 				= intent_message.MOVE_RIGHT,
		[keys.R] 				= custom_intents.STEERING_REQUEST,
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

target_entity_archetype = {
	render = {
		model = crosshair_sprite,
		layer = render_layers.GUI_OBJECTS
	},
	
	transform = {} 
}

target_entity = create_entity(archetyped(target_entity_archetype, {
	render = { model = crosshair_blue } ,
	crosshair = {
			sensitivity = 0
	}
}))
navigation_target_entity = create_entity(archetyped(target_entity_archetype, {}))
forward_navigation_entity = create_entity(archetyped(target_entity_archetype, {}))


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
		player = player.body,
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


flee_behaviour = create_steering_behaviour {
	current_target = target_entity,
	weight = 1,
	behaviour_type = steering_behaviour.FLEE,
	enabled = true,
	erase_when_target_reached = false,
	radius_of_effect = 500,
	force_color = rgba(255, 0, 0, 0)
}
		
seek_archetype = {
	current_target = navigation_target_entity,
	weight = 1,
	behaviour_type = steering_behaviour.SEEK,
	enabled = true,
	erase_when_target_reached = false,
	radius_of_effect = 300,
	force_color = rgba(0, 255, 255, 0)
}			

target_seek_behaviour = create_steering_behaviour (seek_archetype)
forward_seek_behaviour = create_steering_behaviour (archetyped(seek_archetype, {
	current_target = forward_navigation_entity,
	radius_of_effect = 0
}
))

containment_behaviour = create_steering_behaviour {
	weight = 1, 
	behaviour_type = steering_behaviour.CONTAINMENT,
	
	visibility_type = visibility_component.CONTAINMENT,
	ray_count = 100,
	randomize_rays = false,
	only_threats_in_OBB = false,
	
	enabled = true,
	force_color = rgba(0, 255, 255, 255),
	intervention_time_ms = 400,
	avoidance_rectangle_width = 0,
	decision_duration_ms = 0
}

obstacle_avoidance_archetype = {
	weight = 100, 
	behaviour_type = steering_behaviour.OBSTACLE_AVOIDANCE,
	visibility_type = visibility_component.CONTAINMENT,
	
	ray_count = 20,
	randomize_rays = false,
	only_threats_in_OBB = false,
	
	enabled = true,
	force_color = rgba(0, 255, 255, 255),
	intervention_time_ms = 200,
	avoidance_rectangle_width = 0,
	decision_duration_ms = 0,
	ignore_discontinuities_narrower_than = 1
}

wander_behaviour = create_steering_behaviour {
	weight = 0.05, 
	behaviour_type = steering_behaviour.WANDER,
	
	wander_circle_radius = 500,
	wander_circle_distance = 2540,
	wander_displacement_degrees = 5,
	
	enabled = true,
	force_color = rgba(0, 255, 255, 255)
}


obstacle_avoidance_behaviour = create_steering_behaviour (obstacle_avoidance_archetype)
sensor_avoidance_behaviour = create_steering_behaviour (archetyped(obstacle_avoidance_archetype, {
	weight = 0,
	current_target = navigation_target_entity,
	intervention_time_ms = 200
}))

steer_request_fnc = 
			function()
				target_entity.transform.current.pos = player.crosshair.transform.current.pos
					player.body.pathfinding:start_pathfinding(target_entity.transform.current.pos)
					player.body.steering:clear_behaviours()
					
					player.body.steering:add_behaviour(target_seek_behaviour)
					player.body.steering:add_behaviour(forward_seek_behaviour)
					player.body.steering:add_behaviour(sensor_avoidance_behaviour)
					
					player.body.steering:add_behaviour(wander_behaviour)
					player.body.steering:add_behaviour(obstacle_avoidance_behaviour)
			
			end
			
loop_only_info = create_scriptable_info {
	scripted_events = {
		[scriptable_component.LOOP] 	=
			function(message)
				my_atlas:_bind()
				
				local myvel = player.body.physics.body:GetLinearVelocity()
				forward_navigation_entity.transform.current.pos = player.body.transform.current.pos + vec2(myvel.x, myvel.y) * 50
				
				if player.body.pathfinding:is_still_pathfinding() == true then
					navigation_target_entity.transform.current.pos = player.body.pathfinding:get_current_navigation_target()
					target_entity.transform.current.pos = player.body.pathfinding:get_current_target()
					
					obstacle_avoidance_behaviour.enabled = true
					if sensor_avoidance_behaviour.last_output_force:non_zero() then
						target_seek_behaviour.enabled = false
						forward_seek_behaviour.enabled = true
						obstacle_avoidance_behaviour.enabled = true
					else
						target_seek_behaviour.enabled = true
						forward_seek_behaviour.enabled = false
						obstacle_avoidance_behaviour.enabled = false
					end
				else
					target_seek_behaviour.enabled = false
					forward_seek_behaviour.enabled = false
					obstacle_avoidance_behaviour.enabled = false
				end
				
				sensor_avoidance_behaviour.max_intervention_length = (player.body.transform.current.pos - navigation_target_entity.transform.current.pos):length() - 70
				
				--	sensor_avoidance_behaviour.enabled = true
				--	obstacle_avoidance_behaviour.enabled = true
				--forward_seek_behaviour.enabled = true
				
				if obstacle_avoidance_behaviour.last_output_force:non_zero() then
					wander_behaviour.wander_current_angle = obstacle_avoidance_behaviour.last_output_force:get_degrees()
				end
				
				return true
			end,
			
		[scriptable_component.INTENT_MESSAGE] = 
			function(message)
				if message.intent == custom_intents.STEERING_REQUEST then
					steer_request_fnc()
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

create_entity {
	input = {
			custom_intents.STEERING_REQUEST,
			custom_intents.SPEED_INCREASE,
			custom_intents.SPEED_DECREASE
	},
		
	scriptable = {
		available_scripts = loop_only_info
	}
}


set_zoom_level(world_camera)

steer_request_fnc()
target_entity.transform.current.pos = vec2(-300, 1000)
player.body.pathfinding:start_pathfinding(target_entity.transform.current.pos)
