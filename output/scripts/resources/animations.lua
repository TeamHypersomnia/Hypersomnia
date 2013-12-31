npc_size_multiplier = vec2(1, 1)
duration_multiplier = 1

player_legs_animation = create_animation {
	frames = {
		{ model = nil, 			duration_ms = 2.0 },
		{ model = { image = images.player_legs_1,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
		{ model = { image = images.player_legs_2,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
		{ model = { image = images.player_legs_3,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
		{ model = { image = images.player_legs_4,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
		{ model = { image = images.player_legs_5,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
		{ model = { image = images.player_legs_4,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
		{ model = { image = images.player_legs_3,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
		{ model = { image = images.player_legs_2,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
		{ model = { image = images.player_legs_1,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
		{ model = nil, duration_ms = 2.0*duration_multiplier },                                                                    
		{ model = { image = images.player_legs_1,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { image = images.player_legs_2,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { image = images.player_legs_3,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { image = images.player_legs_4,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { image = images.player_legs_5,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { image = images.player_legs_4,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { image = images.player_legs_3,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { image = images.player_legs_2,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { image = images.player_legs_1,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end }
	},
	
	loop_mode = animation.REPEAT
}


player_walk_animation = create_animation {
	frames = {
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_1 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 1) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_2 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 2) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_3 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 3) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_4 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 4) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_5 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 5) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_4 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 4) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_3 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 3) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_2 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 2) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_1 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 1) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_1 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk_cw", 1) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_2 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk_cw", 2) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_3 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk_cw", 3) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_4 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk_cw", 4) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_5 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk_cw", 5) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_4 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk_cw", 4) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_3 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk_cw", 3) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_2 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk_cw", 2) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_1 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk_cw", 1) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end }
	},
	
	loop_mode = animation.REPEAT
}


player_hands_right_animation = create_animation {
	frames = {
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_1 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 1) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_2 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 2) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_3 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 3) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_4 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 4) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_5 }, duration_ms = 65*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 5) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_4 }, duration_ms = 25*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 4) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_3 }, duration_ms = 25*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 3) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_2 }, duration_ms = 25*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 2) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_1 }, duration_ms = 25*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 1) end }
	},
	
	loop_mode = animation.NONE
}

player_hands_left_animation = create_animation {
	frames = {
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_1 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "swing_cw", 1) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_2 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "swing_cw", 2) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_3 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "swing_cw", 3) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_4 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "swing_cw", 4) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_5 }, duration_ms = 65*duration_multiplier, callback = function(subject) set_offsets(subject, "swing_cw", 5) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_4 }, duration_ms = 25*duration_multiplier, callback = function(subject) set_offsets(subject, "swing_cw", 4) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_3 }, duration_ms = 25*duration_multiplier, callback = function(subject) set_offsets(subject, "swing_cw", 3) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_2 }, duration_ms = 25*duration_multiplier, callback = function(subject) set_offsets(subject, "swing_cw", 2) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_1 }, duration_ms = 25*duration_multiplier, callback = function(subject) set_offsets(subject, "swing_cw", 1) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end }
	},
	
	loop_mode = animation.NONE
}

function set_offsets(subject, group, index)
	local npc_info = get_scripted(subject)
	local offset_info = npc_info.wield_offsets[npc_info.current_weapon.animation_index]
	local head_info = npc_info.wield_offsets["HEAD"]
	
	npc_info.head_entity.chase.rotation_orbit_offset = head_info[group][index].pos
	
	if offset_info == nil then return end
	
	npc_info.wielded_entity.chase.rotation_offset = offset_info[group][index].rotation
	npc_info.wielded_entity.chase.rotation_orbit_offset = offset_info[group][index].pos
	
	if offset_info[group][index].flip ~= nil then
		npc_info.wielded_entity.render.flip_horizontally = offset_info[group][index].flip
	else 
		npc_info.wielded_entity.render.flip_horizontally = false
	end
	
end

player_firearm_walk_animation = create_animation {
	frames = {		
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_walk_1, rotation_offset = 30 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 1) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_walk_2, rotation_offset = 30 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 2) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_walk_3, rotation_offset = 30 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 3) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_walk_4, rotation_offset = 30 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 4) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_walk_5, rotation_offset = 30 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 5) end }
	},
	
	loop_mode = animation.INVERSE
}

player_firearm_shot_animation = create_animation {
	frames = {		
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_shot_1, rotation_offset = 30 },  duration_ms = 5*duration_multiplier , callback = function(subject) set_offsets(subject, "shot", 1) end},
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_shot_2, rotation_offset = 30 },  duration_ms = 5*duration_multiplier , callback = function(subject) set_offsets(subject, "shot", 2) end},
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_shot_3, rotation_offset = 30 },  duration_ms = 5*duration_multiplier , callback = function(subject) set_offsets(subject, "shot", 3) end},
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_shot_4, rotation_offset = 30 },  duration_ms = 5*duration_multiplier , callback = function(subject) set_offsets(subject, "shot", 4) end},
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_shot_5, rotation_offset = 30 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "shot", 5) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_shot_4, rotation_offset = 30 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "shot", 4) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_shot_3, rotation_offset = 30 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "shot", 3) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_shot_2, rotation_offset = 30 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "shot", 2) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_shot_1, rotation_offset = 30 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "shot", 1) end }
	},                                                                                                                                                       
	
	loop_mode = animation.NONE
}

player_melee_walk_animation = create_animation {
	frames = {		
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_melee_walk_1 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 1) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_melee_walk_2 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 2) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_melee_walk_3 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 3) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_melee_walk_4 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 4) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_melee_walk_5 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 5) end }
	},
	
	loop_mode = animation.INVERSE
}

player_melee_walk_cw_animation = create_animation {
	frames = {		
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_melee_walk_1 },  duration_ms = 40*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true set_offsets(subject, "walk_cw", 1) end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_melee_walk_2 },  duration_ms = 40*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true set_offsets(subject, "walk_cw", 2) end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_melee_walk_3 },  duration_ms = 40*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true set_offsets(subject, "walk_cw", 3) end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_melee_walk_4 },  duration_ms = 40*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true set_offsets(subject, "walk_cw", 4) end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_melee_walk_5 },  duration_ms = 40*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true set_offsets(subject, "walk_cw", 5) end, callback_out = function(subject) subject.render.flip_vertically = false end }
	},
	
	loop_mode = animation.INVERSE
}


player_melee_hit_animation = create_animation {
	frames = {		
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hit_1},  duration_ms = 30*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 1) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hit_2},  duration_ms = 30*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 2) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hit_3},  duration_ms = 30*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 3) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hit_4},  duration_ms = 30*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 4) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hit_5},  duration_ms = 30*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 5) end }
	},
	
	loop_mode = animation.NONE
}

player_melee_hit_cw_animation = create_animation {
	frames = {		
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hit_1 },  duration_ms = 30*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true set_offsets(subject, "swing_cw", 1) end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hit_2 },  duration_ms = 30*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true set_offsets(subject, "swing_cw", 2) end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hit_3 },  duration_ms = 30*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true set_offsets(subject, "swing_cw", 3) end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hit_4 },  duration_ms = 30*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true set_offsets(subject, "swing_cw", 4) end, callback_out = function(subject) subject.render.flip_vertically = false end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hit_5 },  duration_ms = 30*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true set_offsets(subject, "swing_cw", 5) end, callback_out = function(subject) subject.render.flip_vertically = false end }
	},
	
	loop_mode = animation.NONE
}


player_animation_bare_hands_set = create_animation_set {
	animations = { 
		{ event = animation_events.MOVE_CW, animation_response = player_walk_animation },
		{ event = animation_events.MOVE_CCW, animation_response = player_walk_animation },
		{ event = animation_events.SWING_CW, animation_response = player_hands_left_animation },
		{ event = animation_events.SWING_CCW, animation_response = player_hands_right_animation }
	}
}

player_animation_melee_set = create_animation_set {
	animations = { 
		{ event = animation_events.MOVE_CW, animation_response = player_melee_walk_cw_animation },
		{ event = animation_events.MOVE_CCW, animation_response = player_melee_walk_animation },
		{ event = animation_events.SWING_CW, animation_response = player_melee_hit_cw_animation },
		{ event = animation_events.SWING_CCW, animation_response = player_melee_hit_animation }
	}
}

player_animation_firearm_set = create_animation_set {
	animations = { 
		{ event = animation_events.MOVE_CW, animation_response = player_firearm_walk_animation },
		{ event = animation_events.MOVE_CCW, animation_response = player_firearm_walk_animation },
		{ event = animation_events.SHOT, animation_response = player_firearm_shot_animation },
		{ event = animation_events.SWING_CW, animation_response = player_firearm_shot_animation },
		{ event = animation_events.SWING_CCW, animation_response = player_firearm_shot_animation }
	}
}

player_animation_legs_set = create_animation_set {
	animations = { 
		{ event = animation_events.MOVE, animation_response = player_legs_animation }
	}
}
