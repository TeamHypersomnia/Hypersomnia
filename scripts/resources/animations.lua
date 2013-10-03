legs_animation = create_animation {
	frames = {
		{ model = nil, 			duration_ms = 2.0 },
		{ model = { image = images.legs_5, color = rgba(255, 0, 0, 255) }, duration_ms = 2.0 },
		{ model = { image = images.legs_4, color = rgba(255, 0, 0, 255) }, duration_ms = 2.0 },
		{ model = { image = images.legs_3, color = rgba(255, 0, 0, 255) }, duration_ms = 2.0 },
		{ model = { image = images.legs_2, color = rgba(255, 0, 0, 255) }, duration_ms = 2.0 },
		{ model = { image = images.legs_1, color = rgba(255, 0, 0, 255) }, duration_ms = 2.0 },
		{ model = { image = images.legs_2, color = rgba(255, 0, 0, 255) }, duration_ms = 2.0 },
		{ model = { image = images.legs_3, color = rgba(255, 0, 0, 255) }, duration_ms = 2.0 },
		{ model = { image = images.legs_4, color = rgba(255, 0, 0, 255) }, duration_ms = 2.0 },
		{ model = { image = images.legs_5, color = rgba(255, 0, 0, 255) }, duration_ms = 2.0 },
		{ model = nil, duration_ms = 2.0 },
		{ model = images.legs_6,  duration_ms = 2.0 },
		{ model = images.legs_7,  duration_ms = 2.0 },
		{ model = images.legs_8,  duration_ms = 2.0 },
		{ model = images.legs_9,  duration_ms = 2.0 },
		{ model = images.legs_10, duration_ms = 2.0 },
		{ model = images.legs_9,  duration_ms = 2.0 },
		{ model = images.legs_8,  duration_ms = 2.0 },
		{ model = images.legs_7,  duration_ms = 2.0 },
		{ model = images.legs_6,  duration_ms = 2.0 }
	},
	
	loop_mode = animation.REPEAT
}

player_animation = create_animation {
	frames = {
		{ model = images.player_1,  duration_ms = 2.0 },
		{ model = images.player_2,  duration_ms = 2.0 },
		{ model = images.player_3,  duration_ms = 2.0 },
		{ model = images.player_4,  duration_ms = 2.0 },
		{ model = images.player_5,  duration_ms = 2.0 },
		{ model = images.player_4,  duration_ms = 2.0 },
		{ model = images.player_3,  duration_ms = 2.0 },
		{ model = images.player_2,  duration_ms = 2.0 },
		{ model = images.player_1,  duration_ms = 2.0 },
		{ model = images.player_6,  duration_ms = 2.0 },
		{ model = images.player_7,  duration_ms = 2.0 },
		{ model = images.player_8,  duration_ms = 2.0 },
		{ model = images.player_9,  duration_ms = 2.0 },
		{ model = images.player_10, duration_ms = 2.0 },
		{ model = images.player_8,  duration_ms = 2.0 },
		{ model = images.player_7,  duration_ms = 2.0 },
		{ model = images.player_6,  duration_ms = 2.0 }
	},
	
	loop_mode = animation.REPEAT
}


player_shotgun_animation = create_animation {
	frames = {
		{ model = images.player_shotgun_1,  duration_ms = 5.0 },
		{ model = images.player_shotgun_2,  duration_ms = 5.0 },
		{ model = images.player_shotgun_3,  duration_ms = 5.0 },
		{ model = images.player_shotgun_4,  duration_ms = 5.0 },
		{ model = images.player_shotgun_5,  duration_ms = 5.0 }
	},
	
	loop_mode = animation.INVERSE
}

player_shotgun_shot_animation = create_animation {
	frames = {
		{ model = images.player_shotgun_shot_1,  duration_ms = 2.0 },
		{ model = images.player_shotgun_shot_2,  duration_ms = 2.0 },
		{ model = images.player_shotgun_shot_3,  duration_ms = 2.0 },
		{ model = images.player_shotgun_shot_4,  duration_ms = 2.0 },
		{ model = images.player_shotgun_shot_5,  duration_ms = 2.0 },
		{ model = images.player_shotgun_shot_4,  duration_ms = 2.0 },
		{ model = images.player_shotgun_shot_3,  duration_ms = 2.0 },
		{ model = images.player_shotgun_shot_2,  duration_ms = 2.0 },
		{ model = images.player_shotgun_shot_1,  duration_ms = 2.0 }
	},
	
	loop_mode = animation.REPEAT
}

npc_animation_set = create_animation_set {
	animations = { 
		{ event = animation_events.MOVE, animation_response = player_animation },
		{ event = animation_events.SHOT, animation_response = player_shotgun_shot_animation }
	}
}