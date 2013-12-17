-- improved pathfinding, vertex nodes instead of visible wall tracking
-- introduced exploration
-- improved steering: improved avoidance rectangle
-- obstacle avoidance favors parallellness to velocity instead of distance
-- sensor avoidance
-- discarding narrow discontinuities for navigation in dynamic environments
-- wander steering to avoid equillibriums
-- improved visibility to match the square shape
-- separation, many npcs
-- ai behavior trees: FSMs are bad, reusability of states inside behavior trees (get other weapon)

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

size_mult = vec2(13, -13)
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


target_entity_archetype = {
	--render = {
	--	model = crosshair_sprite,
	--	layer = render_layers.GUI_OBJECTS
	--},
	
	transform = {} 
}

target_entity = create_entity(archetyped(target_entity_archetype, {
	render = { model = crosshair_blue } ,
	crosshair = {
			sensitivity = 0
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
	radius_of_effect = 100,
	force_color = rgba(0, 255, 255, 0)
}			

target_seek_steering = create_steering (seek_archetype)
forward_seek_steering = create_steering (archetyped(seek_archetype, {
	radius_of_effect = 0
}
))

containment_archetype = {
	behaviour_type = containment_behaviour,
	weight = 1, 
	
	ray_filter = filter_obstacle_visibility,
	
	ray_count = 5,
	randomize_rays = true,
	only_threats_in_OBB = false,
	
	force_color = rgba(0, 255, 255, 0),
	intervention_time_ms = 240,
	avoidance_rectangle_width = 0
}

containment_steering = create_steering (containment_archetype) 

obstacle_avoidance_archetype = {
	weight = 1000, 
	behaviour_type = obstacle_avoidance_behaviour,
	visibility_type = visibility_component.DYNAMIC_PATHFINDING,
	
	force_color = rgba(0, 255, 0, 0),
	intervention_time_ms = 200,
	avoidance_rectangle_width = 0,
	ignore_discontinuities_narrower_than = 1
}

wander_steering = create_steering {
	weight = 0.4, 
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

sensor_avoidance_steering = create_steering (archetyped(containment_archetype, {
	weight = 0,
	intervention_time_ms = 200,
	force_color = rgba(0, 0, 255, 0),
	avoidance_rectangle_width = 0
}))

separation_steering = create_steering { 
	behaviour_type = separation_behaviour,
	weight = 1.5,
	force_color = rgba(255, 0, 0, 0),
	
	group = filter_characters_separation,
	square_side = 150,
	field_of_vision_degrees = 240
}

pursuit_steering = create_steering {
	behaviour_type = seek_behaviour,
	weight = 1,
	radius_of_effect = 20,
	force_color = rgba(255, 255, 255, 255),
	
	max_target_future_prediction_ms = 400
}


npc_class = { 
	entity = 0 
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

npc_behaviour_tree = create_behaviour_tree {

	decorators = {
		temporarily_follow_hint = {
			decorator_type = behaviour_timer_decorator,
			maximum_running_time_ms = 50000000
		},
	
		timed_exploration = {
			decorator_type = behaviour_timer_decorator,
			maximum_running_time_ms = 1000
		}
	},
	
	nodes = {
		behave = {
			node_type = behaviour_node.SELECTOR,
			default_return = behaviour_node.SUCCESS
		},
		
		player_visible = {
			on_update = function(entity) 
				local p1 = entity.transform.current.pos
				local p2 = player.body.transform.current.pos
				
				
				ray_output = physics_system:ray_cast(p1, p2, create(b2Filter, filter_obstacle_visibility), entity)
				
				if ray_output.hit and ray_output.what_entity == player.body then
					local npc_info = get_scripted(entity)
					npc_info.was_seen = true
					npc_info.target_entities.last_seen.transform.current.pos = player.body.transform.current.pos
					
					return behaviour_node.SUCCESS 
				end
				
				return behaviour_node.FAILURE
			end
		},
		
		chase_him = {
			on_enter = function(entity)
				entity.pathfinding:clear_pathfinding_info()
				local npc_info = get_scripted(entity)
				npc_info.steering_behaviours.pursuit.target_from:set(player.body)
				npc_info.steering_behaviours.pursuit.enabled = true
				npc_info.steering_behaviours.obstacle_avoidance.enabled = false
				npc_info.steering_behaviours.sensor_avoidance.target_from:set(player.body)
			end,
			
			on_exit = function(entity, status)
				local npc_info = get_scripted(entity)
				npc_info.steering_behaviours.pursuit.enabled = false
				npc_info.steering_behaviours.obstacle_avoidance.enabled = true
				npc_info.steering_behaviours.sensor_avoidance.target_from:set(npc_info.target_entities.navigation)
				
				local player_vel = player.body.physics.body:GetLinearVelocity()
				entity.pathfinding.custom_exploration_hint.origin = player.body.transform.current.pos
				entity.pathfinding.custom_exploration_hint.target = player.body.transform.current.pos + (vec2(player_vel.x, player_vel.y) * 50)
			end
		},
		
		player_seen_before = {
			node_type = behaviour_node.SEQUENCER,
			on_update = function(entity)
				local self = get_scripted(entity)
				
				if self.was_seen then
					return behaviour_node.SUCCESS 
				else
					return behaviour_node.FAILURE
				end
				
				return behaviour_node.FAILURE
			end
		},
		
		
		begin_pathfinding_to_last_seen = {
			default_return = behaviour_node.SUCCESS,
			
			on_enter = function(entity)
				local npc_info = get_scripted(entity)
				entity.pathfinding:start_pathfinding(npc_info.target_entities.last_seen.transform.current.pos)
			end,
			
			on_exit = function(entity, status)
				--entity.pathfinding:clear_pathfinding_info()
			end
		},
		
		go_to_last_seen = {
			on_enter = function(entity)
			--	local npc_info = get_scripted(entity)
			--	npc_info.steering_behaviours.pursuit.target_from:set(npc_info.last_seen)
			--	npc_info.steering_behaviours.pursuit.enabled = true
			end,
			
			on_update = function(entity)
				
				render_system:push_line(debug_line(entity.transform.current.pos, get_scripted(entity).target_entities.last_seen.transform.current.pos, rgba(255, 0, 0, 255)))
				render_system:push_line(debug_line(entity.pathfinding.custom_exploration_hint.origin, entity.pathfinding.custom_exploration_hint.target, rgba(255, 0, 0, 255)))
				
				if entity.pathfinding:is_still_pathfinding() then return behaviour_node.RUNNING end
				return behaviour_node.SUCCESS 
			end
			,
			
			on_exit = function(entity, status)
				entity.pathfinding:clear_pathfinding_info()
			end
		},
		
		follow_hint = {
			decorator_chain = "temporarily_follow_hint",
			
			on_enter = function(entity)
				entity.pathfinding.favor_velocity_parallellness = true
				entity.pathfinding.custom_exploration_hint.enabled = true
				entity.pathfinding:start_exploring()
			end,
			
			on_update = function(entity)
				render_system:push_line(debug_line(entity.pathfinding.custom_exploration_hint.origin, entity.pathfinding.custom_exploration_hint.target, rgba(255, 0, 255, 255)))
				return behaviour_node.RUNNING
			end,
			
			on_exit = function(entity, status)
				entity.pathfinding.custom_exploration_hint.enabled = false
			end
		},
		
		explore = {
			decorator_chain = "timed_exploration",
			
			on_enter = function(entity)
				entity.pathfinding.favor_velocity_parallellness = false
				entity.pathfinding.custom_exploration_hint.enabled = false
			end,
			
			on_exit = function(entity, status)
				--entity.pathfinding:clear_pathfinding_info()
				get_scripted(entity).was_seen = false
			end
			--,
			--
			--on_update = function(entity)
			--	if entity.pathfinding:is_still_pathfinding() then return behaviour_node.RUNNING end
			--	return behaviour_node.SUCCESS 
			--end
		},
		
		idle = {
			default_return = behaviour_node.SUCCESS
		},
		
		walk_around = {
			default_return = behaviour_node.RUNNING,
			
			on_enter = function(entity)
				set_max_speed(entity, 2000)
				entity.pathfinding:start_exploring()
				entity.pathfinding.favor_velocity_parallellness = true
				get_scripted(entity).steering_behaviours.wandering.weight_multiplier = 1.0 
			end,
			
			on_exit = function(entity, status)
				set_max_speed(entity, 5000)
				get_scripted(entity).steering_behaviours.wandering.weight_multiplier = 0.2
			end
		}
	},
	
	connections = {
		behave = {
			"player_visible", "player_seen_before", "idle"
		},
		
		idle = {
			"walk_around"
		},
		
		player_visible = {
			"chase_him"
		},
		
		player_seen_before = {
			"begin_pathfinding_to_last_seen", "go_to_last_seen", "follow_hint", "explore"
		}
	},
	
	root = "behave"
}

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