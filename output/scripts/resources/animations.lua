function create_character_animation_sets(npc_size_multiplier, duration_multiplier, images_table)
	local animations = {}
	local sets = {}
	
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
	
	animations.legs = create_animation {
		frames = {
			{ model = nil, 			duration_ms = 2.0 },
			{ model = { image = images_table.legs_1,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
			{ model = { image = images_table.legs_2,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
			{ model = { image = images_table.legs_3,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
			{ model = { image = images_table.legs_4,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
			{ model = { image = images_table.legs_5,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
			{ model = { image = images_table.legs_4,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
			{ model = { image = images_table.legs_3,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
			{ model = { image = images_table.legs_2,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
			{ model = { image = images_table.legs_1,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
			{ model = nil, duration_ms = 2.0*duration_multiplier },                                                                    
			{ model = { image = images_table.legs_1,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { image = images_table.legs_2,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { image = images_table.legs_3,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { image = images_table.legs_4,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { image = images_table.legs_5,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { image = images_table.legs_4,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { image = images_table.legs_3,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { image = images_table.legs_2,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { image = images_table.legs_1,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end }
		},
		
		loop_mode = animation.REPEAT
	}
	
	
	animations.walk = create_animation {
		frames = {
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.walk_1 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 1) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.walk_2 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 2) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.walk_3 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 3) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.walk_4 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 4) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.walk_5 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 5) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.walk_4 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 4) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.walk_3 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 3) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.walk_2 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 2) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.walk_1 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 1) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.walk_1 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk_cw", 1) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.walk_2 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk_cw", 2) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.walk_3 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk_cw", 3) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.walk_4 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk_cw", 4) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.walk_5 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk_cw", 5) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.walk_4 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk_cw", 4) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.walk_3 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk_cw", 3) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.walk_2 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk_cw", 2) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.walk_1 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "walk_cw", 1) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end }
		},
		
		loop_mode = animation.REPEAT
	}
	
	if images_table.hands_1 ~= nil then
		animations.hands_right = create_animation {
			frames = {
				{ model = { size_multiplier = npc_size_multiplier, image = images_table.hands_1 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 1) end },
				{ model = { size_multiplier = npc_size_multiplier, image = images_table.hands_2 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 2) end },
				{ model = { size_multiplier = npc_size_multiplier, image = images_table.hands_3 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 3) end },
				{ model = { size_multiplier = npc_size_multiplier, image = images_table.hands_4 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 4) end },
				{ model = { size_multiplier = npc_size_multiplier, image = images_table.hands_5 }, duration_ms = 65*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 5) end },
				{ model = { size_multiplier = npc_size_multiplier, image = images_table.hands_4 }, duration_ms = 25*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 4) end },
				{ model = { size_multiplier = npc_size_multiplier, image = images_table.hands_3 }, duration_ms = 25*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 3) end },
				{ model = { size_multiplier = npc_size_multiplier, image = images_table.hands_2 }, duration_ms = 25*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 2) end },
				{ model = { size_multiplier = npc_size_multiplier, image = images_table.hands_1 }, duration_ms = 25*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 1) end }
			},
			
			loop_mode = animation.NONE
		}
		
		animations.hands_left = create_animation {
			frames = {
				{ model = { size_multiplier = npc_size_multiplier, image = images_table.hands_1 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "swing_cw", 1) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
				{ model = { size_multiplier = npc_size_multiplier, image = images_table.hands_2 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "swing_cw", 2) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
				{ model = { size_multiplier = npc_size_multiplier, image = images_table.hands_3 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "swing_cw", 3) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
				{ model = { size_multiplier = npc_size_multiplier, image = images_table.hands_4 }, duration_ms = 20*duration_multiplier, callback = function(subject) set_offsets(subject, "swing_cw", 4) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
				{ model = { size_multiplier = npc_size_multiplier, image = images_table.hands_5 }, duration_ms = 65*duration_multiplier, callback = function(subject) set_offsets(subject, "swing_cw", 5) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
				{ model = { size_multiplier = npc_size_multiplier, image = images_table.hands_4 }, duration_ms = 25*duration_multiplier, callback = function(subject) set_offsets(subject, "swing_cw", 4) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
				{ model = { size_multiplier = npc_size_multiplier, image = images_table.hands_3 }, duration_ms = 25*duration_multiplier, callback = function(subject) set_offsets(subject, "swing_cw", 3) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
				{ model = { size_multiplier = npc_size_multiplier, image = images_table.hands_2 }, duration_ms = 25*duration_multiplier, callback = function(subject) set_offsets(subject, "swing_cw", 2) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end },
				{ model = { size_multiplier = npc_size_multiplier, image = images_table.hands_1 }, duration_ms = 25*duration_multiplier, callback = function(subject) set_offsets(subject, "swing_cw", 1) subject.render.flip_vertically = true end, callback_out = function(subject) subject.render.flip_vertically = false end }
			},
			
			loop_mode = animation.NONE
		}
		
		sets.bare_hands = create_animation_set {
			animations = { 
				{ event = animation_events.MOVE_CW, animation_response = animations.walk },
				{ event = animation_events.MOVE_CCW, animation_response = animations.walk },
				{ event = animation_events.SWING_CW, animation_response = animations.hands_left },
				{ event = animation_events.SWING_CCW, animation_response = animations.hands_right }
			}
		}
	else
		sets.bare_hands = create_animation_set {
			animations = { 
				{ event = animation_events.MOVE_CW, animation_response = animations.walk },
				{ event = animation_events.MOVE_CCW, animation_response = animations.walk }
			}
		}
	end
	
	animations.firearm_walk = create_animation {
		frames = {		
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.firearm_walk_1, rotation_offset = 30 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 1) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.firearm_walk_2, rotation_offset = 30 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 2) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.firearm_walk_3, rotation_offset = 30 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 3) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.firearm_walk_4, rotation_offset = 30 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 4) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.firearm_walk_5, rotation_offset = 30 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 5) end }
		},
		
		loop_mode = animation.INVERSE
	}
	
	animations.firearm_shot = create_animation {
		frames = {		
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.firearm_shot_1, rotation_offset = 30 },  duration_ms = 5*duration_multiplier , callback = function(subject) set_offsets(subject, "shot", 1) end},
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.firearm_shot_2, rotation_offset = 30 },  duration_ms = 5*duration_multiplier , callback = function(subject) set_offsets(subject, "shot", 2) end},
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.firearm_shot_3, rotation_offset = 30 },  duration_ms = 5*duration_multiplier , callback = function(subject) set_offsets(subject, "shot", 3) end},
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.firearm_shot_4, rotation_offset = 30 },  duration_ms = 5*duration_multiplier , callback = function(subject) set_offsets(subject, "shot", 4) end},
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.firearm_shot_5, rotation_offset = 30 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "shot", 5) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.firearm_shot_4, rotation_offset = 30 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "shot", 4) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.firearm_shot_3, rotation_offset = 30 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "shot", 3) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.firearm_shot_2, rotation_offset = 30 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "shot", 2) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.firearm_shot_1, rotation_offset = 30 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "shot", 1) end }
		},                                                                                                                                                       
		
		loop_mode = animation.NONE
	}
	
	animations.melee_walk = create_animation {
		frames = {		
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.melee_walk_1 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 1) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.melee_walk_2 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 2) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.melee_walk_3 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 3) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.melee_walk_4 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 4) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.melee_walk_5 },  duration_ms = 40*duration_multiplier, callback = function(subject) set_offsets(subject, "walk", 5) end }
		},
		
		loop_mode = animation.INVERSE
	}
	
	animations.melee_walk_cw = create_animation {
		frames = {		
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.melee_walk_1 },  duration_ms = 40*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true set_offsets(subject, "walk_cw", 1) end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.melee_walk_2 },  duration_ms = 40*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true set_offsets(subject, "walk_cw", 2) end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.melee_walk_3 },  duration_ms = 40*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true set_offsets(subject, "walk_cw", 3) end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.melee_walk_4 },  duration_ms = 40*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true set_offsets(subject, "walk_cw", 4) end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.melee_walk_5 },  duration_ms = 40*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true set_offsets(subject, "walk_cw", 5) end, callback_out = function(subject) subject.render.flip_vertically = false end }
		},
		
		loop_mode = animation.INVERSE
	}
	
	
	animations.melee_hit = create_animation {
		frames = {		
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.hit_1},  duration_ms = 30*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 1) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.hit_2},  duration_ms = 30*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 2) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.hit_3},  duration_ms = 30*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 3) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.hit_4},  duration_ms = 30*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 4) end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.hit_5},  duration_ms = 30*duration_multiplier, callback = function(subject) set_offsets(subject, "swing", 5) end }
		},
		
		loop_mode = animation.NONE
	}
	
	animations.melee_hit_cw = create_animation {
		frames = {		
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.hit_1 },  duration_ms = 30*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true set_offsets(subject, "swing_cw", 1) end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.hit_2 },  duration_ms = 30*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true set_offsets(subject, "swing_cw", 2) end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.hit_3 },  duration_ms = 30*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true set_offsets(subject, "swing_cw", 3) end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.hit_4 },  duration_ms = 30*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true set_offsets(subject, "swing_cw", 4) end, callback_out = function(subject) subject.render.flip_vertically = false end },
			{ model = { size_multiplier = npc_size_multiplier, image = images_table.hit_5 },  duration_ms = 30*duration_multiplier, callback = function(subject) subject.render.flip_vertically = true set_offsets(subject, "swing_cw", 5) end, callback_out = function(subject) subject.render.flip_vertically = false end }
		},
		
		loop_mode = animation.NONE
	}
	
	sets.melee = create_animation_set {
		animations = { 
			{ event = animation_events.MOVE_CW, animation_response = animations.melee_walk_cw },
			{ event = animation_events.MOVE_CCW, animation_response = animations.melee_walk },
			{ event = animation_events.SWING_CW, animation_response = animations.melee_hit_cw },
			{ event = animation_events.SWING_CCW, animation_response = animations.melee_hit }
		}
	}
	
	sets.firearm = create_animation_set {
		animations = { 
			{ event = animation_events.MOVE_CW, animation_response = animations.firearm_walk },
			{ event = animation_events.MOVE_CCW, animation_response = animations.firearm_walk },
			{ event = animation_events.SHOT, animation_response = animations.firearm_shot },
			{ event = animation_events.SWING_CW, animation_response = animations.firearm_shot },
			{ event = animation_events.SWING_CCW, animation_response = animations.firearm_shot }
		}
	}
	
	sets.legs = create_animation_set {
		animations = { 
			{ event = animation_events.MOVE, animation_response = animations.legs }
		}
	}
	
	local output = {}
	output.sets = sets
	output.animations = animations
	
	return output
end

player_animations = create_character_animation_sets(vec2(1, 1), 1, player_images)


enemy_animations = create_character_animation_sets(vec2(1, 1), 1, enemy_images)
