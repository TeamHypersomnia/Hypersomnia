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