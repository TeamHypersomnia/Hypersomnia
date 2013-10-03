render_layers = {
	GUI_OBJECTS = 0,
	EFFECTS = 1,
	OBJECTS = 2,
	PLAYERS = 3,
	BULLETS = 4,
	LEGS = 5,
	ON_GROUND = 6,
	GROUND = 7
}

my_npc_archetype = {
	transform = { 
		pos = vec2(0, 0),
		rotation = 0
	},
	
	render = {
		layer = render_layers.PLAYERS,
		mask = render_component.WORLD,
		model = nil
	},
	
	animate = {
		available_animations = npc_animation_set
	}
}

npc_1 = create_entity_from_entry {
	archetype = my_npc_archetype,
	
	transform = {
		pos = vec2(100, 100),
		rotation = 0
	}
}

npc_2 = create_entity_from_entry {
	archetype = my_npc_archetype,
	
	transform = {
		pos = vec2(500, 100),
		rotation = 180
	}
}

blue_crate_sprite = create_sprite {
	image = images.crate_texture,
	color = rgba(0, 0, 255, 255),
	size = vec2(200, 200)
}

crate_archetype = {
	transform = {
		pos = vec2(0, 0),
		rotation = 0
	},
	
	render = {
		layer = render_layers.OBJECTS,
		mask = render_component.WORLD,
		model = blue_crate_sprite
	}
}

create_entity_from_entry {
	archetype = crate_archetype,
	
	transform = {
		pos = vec2(250, 230),
		rotation = 10
	}
}

create_options {"CHARACTERS", "OBJECTS", "BULLETS", "CORPSES" }

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

msg = create(animate_message, {
	set_animation = nil,
	animation_type = animation_events.SHOT,
	preserve_state_if_animation_changes = false,
	message_type = animate_message.START,
	change_animation = true,
	change_speed = true,
	speed_factor = 0.05,
	animation_priority = 0
})

msg.subject = npc_1
world:post_message(msg)

msg.subject = npc_2
world:post_message(msg)

--
--create_entity_from_entry (crate_archetype)
--create_entity_from_entry {
--	archetype = crate_archetype,
--	
--	transform = {
--		pos = vec2(200, 100),
--		rotation = 40
--	}
--}


--
--local group_archetype = {
--	body = {
--		archetype = crate_archetype,
--		
--		transform = {
--			pos = vec2(100, 100),
--			rotation = 20
--		}
--	},
--	
--	chaser = {
--		archetype = crate_archetype,
--		
--		chase = {
--			target = 'body',
--			relative = false,
--			offset = vec2(100, 0),
--			type = chase_component.OFFSET,
--			rotation_orbit_offset = vec2(0, 0),
--			rotation_offset = 45,
--			chase_rotation = true,
--			track_origin = false
--		}
--	}
--}
--
--create_entity_group {
--	archetype = group_archetype,
--	
--	body = {
--		transform = {
--			pos = vec2(214, 520),
--			rotation = 0
--		}	
--	}
--}

world_camera = create_entity_from_entry {
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


