pollock_sprite = create_sprite {
	image = images.pollock,
	size = vec2(2000, 1000)
}

bg = create_entity {
	render = {
		model = pollock_sprite,
		mask = render_component.WORLD,
		layer = render_layers.GROUND
	},
	
	transform = {
		pos	= vec2(0, 300),
		rotation = 0
	}
}

filter_objects = b2Filter()
filter_characters = b2Filter()
filter_bullets = b2Filter()
filter_corpses = b2Filter()

filter_characters.categoryBits = CHARACTERS;
filter_characters.maskBits = bitor(OBJECTS, BULLETS, CHARACTERS)

filter_objects.categoryBits = OBJECTS;
filter_objects.maskBits = bitor(OBJECTS, BULLETS, CHARACTERS, CORPSES);

filter_bullets.categoryBits = BULLETS;
filter_bullets.maskBits = bitor(OBJECTS, CHARACTERS);

filter_corpses.categoryBits = CORPSES;
filter_corpses.maskBits = OBJECTS;


main_context = create_input_context {
	intents = { 
		[mouse.raw_motion] 		= intent_message.MOVE_FORWARD,
		[keys.W] 				= intent_message.MOVE_FORWARD,
		[keys.S] 				= intent_message.MOVE_BACKWARD,
		[keys.A] 				= intent_message.MOVE_LEFT,
		[keys.D] 				= intent_message.MOVE_RIGHT,
		[mouse.ldoubleclick] 	= intent_message.SHOOT,
		[mouse.ltripleclick] 	= intent_message.SHOOT,
		[mouse.ldown] 			= intent_message.SHOOT,
		[mouse.rdown] 			= intent_message.SWITCH_LOOK,
		[mouse.rdoubleclick] 	= intent_message.SWITCH_LOOK
	}
}

input_system:add_context(main_context)

world_camera = create_entity {
	transform = {
		pos = vec2(),
		rotation = 0
	},

	camera = {
		enabled = true,
		
		layer = 0, -- 0 = topmost
		mask = render_component.WORLD,
		screen_rect = rect_xywh(0, 0, 800, 800),
		ortho = rect_ltrb(0, 0, 800, 800),
		
		enable_smoothing = true,
		smoothing_average_factor = 0.004,
		averages_per_sec = 60,
	
		crosshair = nil,
		player = nil,
		orbit_mode = camera_component.ANGLED,
		max_look_expand = vec2(),
		angled_look_length = 0
	}
}


