modes = {
	DUNGEON = 1, SMALL_DYNAMIC = 2
}

current_mode = modes.DUNGEON

physics_system.timestep_multiplier = 1
physics_system.enable_interpolation = 0

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

render_system.draw_steering_forces = 0
render_system.draw_substeering_forces = 0
render_system.draw_velocities = 0

render_system.draw_avoidance_info = 0
render_system.draw_wandering_info = 0

render_system.visibility_expansion = 1.0
render_system.max_visibility_expansion_distance = 1
render_system.draw_visibility = 1

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

function map_uv_square(texcoords_to_map, texture_to_map)
	local lefttop = vec2()
	local bottomright = vec2()
	
	for i = 0, texcoords_to_map:get_vertex_count()-1 do
		local v = texcoords_to_map:get_vertex(i).pos
		if v.x < lefttop.x then lefttop.x = v.x end
		if v.y < lefttop.y then lefttop.y = v.y end
		if v.x > bottomright.x then bottomright.x = v.x end
		if v.y > bottomright.y then bottomright.y = v.y end
	end
	print(lefttop.x, lefttop.y)
	print(bottomright.x, bottomright.y)
	
	for i = 0, texcoords_to_map:get_vertex_count()-1 do
		local v = texcoords_to_map:get_vertex(i)
		v:set_texcoord (vec2(
		(v.pos.x - lefttop.x) / (bottomright.x-lefttop.x),
		(v.pos.y - lefttop.y) / (bottomright.y-lefttop.y)
		), texture_to_map)
		
		--print(v.texcoord.x, v.texcoord.y)
	end
end

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

if current_mode == modes.SMALL_DYNAMIC then
	size_mult = vec2(80, -80)*3.33
elseif current_mode == modes.DUNGEON then
	size_mult = vec2(13, -13)
end

map_points = {
	square = {
		U = archetyped(point_archetype, { pos = vec2(-5.1, 6.29) * size_mult, color = rgba(0, 150, 150, 255) }),
		V = archetyped(point_archetype, { pos = vec2(-5.16, -4.68) * size_mult, color = rgba(0, 150, 150, 255) }),
		W = archetyped(point_archetype, { pos = vec2(8.19, -4.39) * size_mult,  color = rgba(0, 150, 150, 255) }),
		Z = archetyped(point_archetype, { pos = vec2(8.19, 6.87) * size_mult, color = rgba(0, 150, 150, 255) })
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

if current_mode == modes.SMALL_DYNAMIC then
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
end

my_dungeon = {

vec2(440.3286, 74.7106 ) * size_mult,
vec2(490.8061, 97.9029 ) * size_mult,
vec2(477.8457, 177.7119) * size_mult,
vec2(405.5401, 208.4077) * size_mult,
vec2(398.7188, 259.5673) * size_mult,
vec2(483.3027, 242.5141) * size_mult,
vec2(520.1376, 224.7788) * size_mult,
vec2(525.5947, 289.5810) * size_mult,
vec2(481.2563, 329.1444) * size_mult,
vec2(434.1895, 327.7801) * size_mult,
vec2(408.9507, 289.5810) * size_mult,
vec2(375.5265, 302.5414) * size_mult,
vec2(369.3873, 338.0121) * size_mult,
vec2(380.3014, 371.4363) * size_mult,
vec2(439.6465, 353.0189) * size_mult,
vec2(477.8457, 351.6546) * size_mult,
vec2(492.1704, 387.1253) * size_mult,
vec2(477.1636, 411.6819) * size_mult,
vec2(420.5469, 426.0066) * size_mult,
vec2(370.0694, 409.6355) * size_mult,
vec2(338.6915, 398.7215) * size_mult,
vec2(323.6847, 411.6819) * size_mult,
vec2(327.7775, 438.2849) * size_mult,
vec2(357.7911, 443.0598) * size_mult,
vec2(376.8907, 463.5237) * size_mult,
vec2(393.9439, 493.5373) * size_mult,
vec2(431.4610, 485.3518) * size_mult,
vec2(459.4282, 443.0598) * size_mult,
vec2(482.6206, 455.3381) * size_mult,
vec2(491.4883, 496.2658) * size_mult,
vec2(482.6206, 527.6437) * size_mult,
vec2(426.0039, 518.7760) * size_mult,
vec2(371.4337, 518.7760) * size_mult,
vec2(344.1486, 487.3981) * size_mult,
vec2(327.7775, 464.2058) * size_mult,
vec2(304.5851, 468.9807) * size_mult,
vec2(300.4924, 513.3190) * size_mult,
vec2(273.8893, 522.1867) * size_mult,
vec2(233.6438, 520.1403) * size_mult,
vec2(192.7161, 515.3654) * size_mult,
vec2(158.6097, 516.7297) * size_mult,
vec2(124.5033, 511.2726) * size_mult,
vec2(93.1254, 491.4909 ) * size_mult,
vec2(73.3436, 453.2917 ) * size_mult,
vec2(74.0258, 408.9534 ) * size_mult,
vec2(97.2181, 403.4964 ) * size_mult,
vec2(115.6356, 441.6956) * size_mult,
vec2(124.5033, 479.2126) * size_mult,
vec2(172.2522, 480.5769) * size_mult,
vec2(239.1008, 471.7092) * size_mult,
vec2(236.3723, 432.8279) * size_mult,
vec2(179.0735, 434.8743) * size_mult,
vec2(173.6165, 388.4895) * size_mult,
vec2(241.1472, 383.0325) * size_mult,
vec2(265.7038, 419.8674) * size_mult,
vec2(295.0353, 406.9070) * size_mult,
vec2(288.2140, 355.0653) * size_mult,
vec2(252.0612, 311.4091) * size_mult,
vec2(204.3123, 325.7338) * size_mult,
vec2(158.6097, 368.7078) * size_mult,
vec2(116.3177, 372.8006) * size_mult,
vec2(101.9930, 327.0980) * size_mult,
vec2(76.7543, 299.8129 ) * size_mult,
vec2(95.8539, 239.7856 ) * size_mult,
vec2(159.2918, 235.6928) * size_mult,
vec2(143.6029, 291.6273) * size_mult,
vec2(214.5442, 295.0380) * size_mult,
vec2(223.4119, 232.2822) * size_mult,
vec2(288.8962, 208.4077) * size_mult,
vec2(303.2209, 261.6137) * size_mult,
vec2(292.9889, 285.4882) * size_mult,
vec2(316.1813, 332.5550) * size_mult,
vec2(335.9630, 358.4759) * size_mult,
vec2(334.5988, 289.5810) * size_mult,
vec2(357.7911, 218.6396) * size_mult,
vec2(320.2741, 200.2222) * size_mult,
vec2(284.1213, 159.9766) * size_mult,
vec2(232.2795, 191.3545) * size_mult,
vec2(196.1267, 228.8715) * size_mult,
vec2(162.0203, 184.5332) * size_mult,
vec2(101.9930, 180.4404) * size_mult,
vec2(82.8934, 165.4336 ) * size_mult,
vec2(84.9398, 134.0557 ) * size_mult,
vec2(119.0462, 98.5850 ) * size_mult,
vec2(157.2454, 149.0625) * size_mult,
vec2(192.7161, 126.5523) * size_mult,
vec2(164.0667, 93.8101 ) * size_mult,
vec2(167.4773, 73.3463 ) * size_mult,
vec2(132.0662, 72.2483) * size_mult,
vec2(73.2964, 70.4260 ) * size_mult,
vec2(71.9794, 187.2617 ) * size_mult,
vec2(121.0926, 207.7256) * size_mult,
vec2(80.1649, 222.0503 ) * size_mult,
vec2(56.2904, 224.7788 ) * size_mult,
vec2(39.9194, 181.1226 ) * size_mult,
vec2(39.2372, 83.5782  ) * size_mult,
vec2(50.8334, 59.7037  ) * size_mult,
vec2(134.7352, 44.6969 ) * size_mult,
vec2(196.8089, 39.9220 ) * size_mult,
vec2(196.1267, 77.4391 ) * size_mult,
vec2(224.0940, 108.1348) * size_mult,
vec2(232.9617, 141.5591) * size_mult,
vec2(282.7570, 86.9889 ) * size_mult,
vec2(322.3205, 66.5250 ) * size_mult,
vec2(338.6915, 135.4200) * size_mult,
vec2(350.2877, 164.7515) * size_mult,
vec2(391.2154, 147.6983) * size_mult,
vec2(427.3682, 155.8838) * size_mult,
vec2(404.1758, 112.9097) * size_mult,
vec2(372.7979, 111.5455) * size_mult,
vec2(353.0162, 71.2999 ) * size_mult,
vec2(380.3014, 56.2931 ) * size_mult,
vec2(428.7325, 56.9752 ) * size_mult,
vec2(417.8184, 91.0816 ) * size_mult,
vec2(440.3286, 74.7106 ) * size_mult
}

if current_mode == modes.DUNGEON then
environment_poly = create_polygon_with_holes {
	subject = {
vec2(45.0325, 535.6137) * size_mult,
vec2(53.9025, 453.0542) * size_mult,
vec2(66.1841, 369.8123) * size_mult,
vec2(52.5379, 327.5090) * size_mult,
vec2(36.1625, 274.9711) * size_mult,
vec2(23.8809, 210.8339) * size_mult,
vec2(25.2455, 137.8267) * size_mult,
vec2(19.1047, 87.3357 ) * size_mult,
vec2(31.3863, 53.9025 ) * size_mult,
vec2(71.6426, 22.5162 ) * size_mult,
vec2(154.8845, 13.6462 ) * size_mult,
vec2(230.6209, 17.7401 ) * size_mult,
vec2(249.0433, 57.9964 ) * size_mult,
vec2(312.4982, 45.7148 ) * size_mult,
vec2(409.3863, 27.2924 ) * size_mult,
vec2(484.4404, 21.8339 ) * size_mult,
vec2(524.0144, 55.2671 ) * size_mult,
vec2(524.6968, 113.9458) * size_mult,
vec2(530.1552, 180.8123) * size_mult,
vec2(489.2166, 208.1047) * size_mult,
vec2(540.3899, 219.7040) * size_mult,
vec2(551.9892, 278.3827) * size_mult,
vec2(544.4838, 337.7437) * size_mult,
vec2(519.9206, 375.9531) * size_mult,
vec2(504.2274, 417.5740) * size_mult,
vec2(505.5921, 451.6895) * size_mult,
vec2(557.4477, 474.2058) * size_mult,
vec2(539.0253, 516.5090) * size_mult,
vec2(508.3213, 539.7076) * size_mult,
vec2(421.6679, 549.9422) * size_mult,
vec2(281.7942, 549.9422) * size_mult,
vec2(73.0072, 546.5307) * size_mult, 
vec2(45.0325, 535.6137) * size_mult
	},
	
	holes = {
		my_dungeon
	}
}
end

ground_poly = create_polygon_with_holes {
	subject = my_dungeon,
	
	holes = {
	
	}
}

map_uv_square(environment_poly, images.metal)
map_uv_square(ground_poly, images.background)


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

if current_mode == modes.SMALL_DYNAMIC then
create_entity (archetyped(small_box_archetype, {
		transform = {
			pos = vec2(100, -600) + vec2(-1000, 1000),
			rotation = 0
		}
}))

create_entity (archetyped(small_box_archetype, {
		transform = {
			pos = vec2(400, -432) + vec2(-1000, 1000),
			rotation = 20
		}
}))

create_entity (archetyped(small_box_archetype, {
		transform = {
			pos = vec2(800, -432) + vec2(-1000, 1000),
			rotation = -30
		}
}))

create_entity (archetyped(big_box_archetype, {
		transform = {
			pos = vec2(200+(-170), 340) + vec2(-1000, 700),
			rotation = 0
		}
}))

create_entity (archetyped(big_box_archetype, {
		transform = {
			pos = vec2(200+(-500), 300) + vec2(-1000, 700),
			rotation = 0
		}
}))
	
create_entity (archetyped(big_box_archetype, {
	transform = {
		pos = vec2((-280), 500) + vec2(-1000, 700),
		rotation = 0
	}
}))
end

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
					ignore_discontinuities_shorter_than = 150,
					color = rgba(255, 0, 255, 0),
					filter = filter_obstacle_visibility
				},	
				
				[visibility_component.DYNAMIC_PATHFINDING] = {
					square_side = 7000,
					color = rgba(0, 255, 255, 120),
					ignore_discontinuities_shorter_than = 150,
					filter = filter_pathfinding_visibility
				}
			}
		},
		
		pathfinding = {
			enable_backtracking = true,
			target_offset = 100,
			rotate_navpoints = 10,
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
		[keys.ESC] 				= custom_intents.QUIT,
		[keys.S] 				= intent_message.MOVE_BACKWARD,
		[keys.A] 				= intent_message.MOVE_LEFT,
		[keys.D] 				= intent_message.MOVE_RIGHT,
		[keys.R] 				= custom_intents.STEERING_REQUEST,
		[keys.E] 				= custom_intents.EXPLORING_REQUEST,
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
		player = player.body
	},
	
	input = {
		intent_message.SWITCH_LOOK,
		custom_intents.ZOOM_CAMERA
	},
	
	--chase = {
	--	target = player.body
	--},
	
	scriptable = {
		available_scripts = scriptable_zoom
	}
}))


flee_steering = create_steering {
	behaviour_type = flee_behaviour,
	weight = 1,
	radius_of_effect = 500,
	force_color = rgba(255, 0, 0, 0)
}
		
seek_archetype = {
	behaviour_type = seek_behaviour,
	weight = 1,
	radius_of_effect = 20,
	force_color = rgba(0, 255, 255, 0)
}			

target_seek_steering = create_steering (seek_archetype)
forward_seek_steering = create_steering (archetyped(seek_archetype, {
	radius_of_effect = 0
}
))

containment_steering = create_steering {
	behaviour_type = containment_behaviour,
	weight = 1, 
	
	visibility_type = visibility_component.CONTAINMENT,
	ray_count = 100,
	randomize_rays = false,
	only_threats_in_OBB = false,
	
	force_color = rgba(0, 255, 255, 0),
	intervention_time_ms = 400,
	avoidance_rectangle_width = 0
}

obstacle_avoidance_archetype = {
	weight = 100, 
	behaviour_type = obstacle_avoidance_behaviour,
	visibility_type = visibility_component.CONTAINMENT,
	
	ray_count = 20,
	randomize_rays = false,
	only_threats_in_OBB = false,
	
	force_color = rgba(0, 255, 0, 255),
	intervention_time_ms = 200,
	avoidance_rectangle_width = 0,
	ignore_discontinuities_narrower_than = 1
}

wander_steering = create_steering {
	weight = 1, 
	behaviour_type = wander_behaviour,
	
	circle_radius = 2000,
	circle_distance = 2540,
	displacement_degrees = 15,
	
	force_color = rgba(0, 255, 255, 0)
}

obstacle_avoidance_steering = create_steering (archetyped(obstacle_avoidance_archetype, {
	navigation_seek = target_seek_steering,
	navigation_correction = containment_steering
}))

sensor_avoidance_steering = create_steering (archetyped(obstacle_avoidance_archetype, {
	weight = 0,
	intervention_time_ms = 200,
	force_color = rgba(0, 0, 255, 255)
}))

behaviour_state(target_seek_steering)
player_behaviours = {
	target_seeking = behaviour_state(target_seek_steering),
	forward_seeking = behaviour_state(forward_seek_steering),
	
	sensor_avoidance = behaviour_state(sensor_avoidance_steering),
	wandering = behaviour_state(wander_steering),
	obstacle_avoidance = behaviour_state(obstacle_avoidance_steering)
}

player_behaviours.forward_seeking.target_from:set(forward_navigation_entity)
player_behaviours.target_seeking.target_from:set(navigation_target_entity)
player_behaviours.sensor_avoidance.target_from:set(navigation_target_entity)
player_behaviours.wandering.enabled = true

steer_request_fnc = 
			function()
				target_entity.transform.current.pos = player.crosshair.transform.current.pos
					player.body.steering:clear_behaviours()
					
					for k, v in pairs(player_behaviours) do
						player.body.steering:add_behaviour(v)
					end
			
			end
			
loop_only_info = create_scriptable_info {
	scripted_events = {
		[scriptable_component.LOOP] 	=
			function(message)
				my_atlas:_bind()
				
				local myvel = player.body.physics.body:GetLinearVelocity()
				forward_navigation_entity.transform.current.pos = player.body.transform.current.pos + vec2(myvel.x, myvel.y) * 50
				
				if player.body.pathfinding:is_still_pathfinding() or player.body.pathfinding:is_still_exploring() then
					navigation_target_entity.transform.current.pos = player.body.pathfinding:get_current_navigation_target()
					target_entity.transform.current.pos = player.body.pathfinding:get_current_target()
					
					player_behaviours.obstacle_avoidance.enabled = true
					if player_behaviours.sensor_avoidance.last_output_force:non_zero() then
						player_behaviours.target_seeking.enabled = false
						player_behaviours.forward_seeking.enabled = true
						player_behaviours.obstacle_avoidance.enabled = true
					else
						player_behaviours.target_seeking.enabled = true
						player_behaviours.forward_seeking.enabled = false
						--player_behaviours.obstacle_avoidance.enabled = false
					end
				else
					player_behaviours.target_seeking.enabled = false
					--player_behaviours.forward_seeking.enabled = false
					--player_behaviours.obstacle_avoidance.enabled = false
				end
				
				player_behaviours.sensor_avoidance.max_intervention_length = (player.body.transform.current.pos - navigation_target_entity.transform.current.pos):length() - 70
				
				--	player_behaviours.sensor_avoidance.enabled = true
				--	player_behaviours.obstacle_avoidance.enabled = true
				player_behaviours.forward_seeking.enabled = true
				
				if player_behaviours.obstacle_avoidance.last_output_force:non_zero() then
					player_behaviours.wandering.current_wander_angle = player_behaviours.obstacle_avoidance.last_output_force:get_degrees()
				end
				
				return true
			end,
			
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
			custom_intents.EXPLORING_REQUEST,
			custom_intents.SPEED_INCREASE,
			custom_intents.SPEED_DECREASE,
			custom_intents.QUIT
	},
		
	scriptable = {
		available_scripts = loop_only_info
	}
}


set_zoom_level(world_camera)

--steer_request_fnc()
target_entity.transform.current.pos = vec2(-300, 1000)
player.body.pathfinding:start_pathfinding(target_entity.transform.current.pos)
