local default_transform = {
	transform = {
		pos = vec2(300, 200),
		rotation = 0
	}
}

local crate_archetype = {
	archetype = default_transform, 
	
	render = {
		image = crate_sprite,
		layer = 0,
		mask = 0
	}
}

local group_archetype = {
	body = {
		archetype = crate_archetype,
		
		transform = {
			pos = vec2(100, 100),
			rotation = 20
		}
	},
	
	chaser = {
		archetype = crate_archetype,
		
		chase = {
			target = 'body',
			relative = false,
			offset = vec2(100, 0),
			type = chase_component.OFFSET,
			rotation_orbit_offset = vec2(0, 0),
			rotation_offset = 45,
			chase_rotation = true,
			track_origin = false
		}
	}
}

create_entity_group {
	archetype = group_archetype,
	
	body = {
		transform = {
			pos = vec2(214, 520),
			rotation = 0
		}	
	}
}

local world_camera = create_entity_from_entry {
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
