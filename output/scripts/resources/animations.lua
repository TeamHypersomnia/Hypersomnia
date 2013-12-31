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
		{ model = { image = images.player_legs_6,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
		{ model = { image = images.player_legs_7,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
		{ model = { image = images.player_legs_8,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
		{ model = { image = images.player_legs_9,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
		{ model = { image = images.player_legs_10, size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
		{ model = { image = images.player_legs_9,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
		{ model = { image = images.player_legs_8,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
		{ model = { image = images.player_legs_7,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier },
		{ model = { image = images.player_legs_6,  size_multiplier = npc_size_multiplier }, duration_ms = 20*duration_multiplier }
	},
	
	loop_mode = animation.REPEAT
}

--enemy_legs_animation = create_animation {
--	frames = {
--		{ model = nil, 			duration_ms = 2.0 },
--		{ model = { image = images.legs_5,  size_multiplier = vec2(2, 2)*npc_size_multiplier }, duration_ms = 20*duration_multiplier },
--		{ model = { image = images.legs_4,  size_multiplier = vec2(2, 2)*npc_size_multiplier }, duration_ms = 20*duration_multiplier },
--		{ model = { image = images.legs_3,  size_multiplier = vec2(2, 2)*npc_size_multiplier }, duration_ms = 20*duration_multiplier },
--		{ model = { image = images.legs_2,  size_multiplier = vec2(2, 2)*npc_size_multiplier }, duration_ms = 20*duration_multiplier },
--		{ model = { image = images.legs_1,  size_multiplier = vec2(2, 2)*npc_size_multiplier }, duration_ms = 20*duration_multiplier },
--		{ model = { image = images.legs_2,  size_multiplier = vec2(2, 2)*npc_size_multiplier }, duration_ms = 20*duration_multiplier },
--		{ model = { image = images.legs_3,  size_multiplier = vec2(2, 2)*npc_size_multiplier }, duration_ms = 20*duration_multiplier },
--		{ model = { image = images.legs_4,  size_multiplier = vec2(2, 2)*npc_size_multiplier }, duration_ms = 20*duration_multiplier },
--		{ model = { image = images.legs_5,  size_multiplier = vec2(2, 2)*npc_size_multiplier }, duration_ms = 20*duration_multiplier },
--		{ model = nil, duration_ms = 2.0*duration_multiplier },                                                                    
--		{ model = { image = images.legs_6,  size_multiplier = vec2(2, 2)*npc_size_multiplier }, duration_ms = 20*duration_multiplier },
--		{ model = { image = images.legs_7,  size_multiplier = vec2(2, 2)*npc_size_multiplier }, duration_ms = 20*duration_multiplier },
--		{ model = { image = images.legs_8,  size_multiplier = vec2(2, 2)*npc_size_multiplier }, duration_ms = 20*duration_multiplier },
--		{ model = { image = images.legs_9,  size_multiplier = vec2(2, 2)*npc_size_multiplier }, duration_ms = 20*duration_multiplier },
--		{ model = { image = images.legs_10, size_multiplier = vec2(2, 2)*npc_size_multiplier }, duration_ms = 20*duration_multiplier },
--		{ model = { image = images.legs_9,  size_multiplier = vec2(2, 2)*npc_size_multiplier }, duration_ms = 20*duration_multiplier },
--		{ model = { image = images.legs_8,  size_multiplier = vec2(2, 2)*npc_size_multiplier }, duration_ms = 20*duration_multiplier },
--		{ model = { image = images.legs_7,  size_multiplier = vec2(2, 2)*npc_size_multiplier }, duration_ms = 20*duration_multiplier },
--		{ model = { image = images.legs_6,  size_multiplier = vec2(2, 2)*npc_size_multiplier }, duration_ms = 20*duration_multiplier }
--	},
--	
--	loop_mode = animation.REPEAT
--}

player_walk_animation = create_animation {
	frames = {
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_1 }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_2 }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_3 }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_4 }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_5 }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_4 }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_3 }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_2 }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_1 }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_6 }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_7 }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_8 }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_9 }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_10}, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_9 }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_8 }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_7 }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_walk_6 }, duration_ms = 20*duration_multiplier }
	},
	
	loop_mode = animation.REPEAT
}

--enemy_animation = create_animation {
--	frames = {
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_1 }, duration_ms = 20*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_2 }, duration_ms = 20*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_3 }, duration_ms = 20*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_4 }, duration_ms = 20*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_5 }, duration_ms = 20*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_4 }, duration_ms = 20*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_3 }, duration_ms = 20*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_2 }, duration_ms = 20*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_1 }, duration_ms = 20*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_6 }, duration_ms = 20*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_7 }, duration_ms = 20*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_8 }, duration_ms = 20*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_9 }, duration_ms = 20*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_10}, duration_ms = 20*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_9 }, duration_ms = 20*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_8 }, duration_ms = 20*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_7 }, duration_ms = 20*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_6 }, duration_ms = 20*duration_multiplier }
--	},
--	
--	loop_mode = animation.REPEAT
--}


player_hands_right_animation = create_animation {
	frames = {
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_1 }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_2 }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_3 }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_4 }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_5 }, duration_ms = 65*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_4 }, duration_ms = 25*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_3 }, duration_ms = 25*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_2 }, duration_ms = 25*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_1 }, duration_ms = 25*duration_multiplier }
	},
	
	loop_mode = animation.NONE
}

player_hands_left_animation = create_animation {
	frames = {
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_6  }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_7  }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_8  }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_9  }, duration_ms = 20*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_10 }, duration_ms = 65*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_9  }, duration_ms = 25*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_8  }, duration_ms = 25*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_7  }, duration_ms = 25*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hands_6  }, duration_ms = 25*duration_multiplier }
	},
	
	loop_mode = animation.NONE
}

player_firearm_walk_animation = create_animation {
	frames = {		
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_walk_1},  duration_ms = 40*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_walk_2},  duration_ms = 40*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_walk_3},  duration_ms = 40*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_walk_4},  duration_ms = 40*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_walk_5},  duration_ms = 40*duration_multiplier }
	},
	
	loop_mode = animation.INVERSE
}

player_firearm_shot_animation = create_animation {
	frames = {		
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_shot_1},  duration_ms = 5*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_shot_2},  duration_ms = 5*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_shot_3},  duration_ms = 5*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_shot_4},  duration_ms = 5*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_shot_5},  duration_ms = 40*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_shot_4},  duration_ms = 40*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_shot_3},  duration_ms = 40*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_shot_2},  duration_ms = 40*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_firearm_shot_1},  duration_ms = 40*duration_multiplier }
	},
	
	loop_mode = animation.NONE
}

player_melee_walk_animation = create_animation {
	frames = {		
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_melee_walk_1},  duration_ms = 40*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_melee_walk_2},  duration_ms = 40*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_melee_walk_3},  duration_ms = 40*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_melee_walk_4},  duration_ms = 40*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_melee_walk_5},  duration_ms = 40*duration_multiplier }
	},
	
	loop_mode = animation.INVERSE
}

player_melee_walk_cw_animation = create_animation {
	frames = {		
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_melee_walk_1cw},  duration_ms = 40*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_melee_walk_2cw},  duration_ms = 40*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_melee_walk_3cw},  duration_ms = 40*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_melee_walk_4cw},  duration_ms = 40*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_melee_walk_5cw},  duration_ms = 40*duration_multiplier }
	},
	
	loop_mode = animation.INVERSE
}


player_melee_hit_animation = create_animation {
	frames = {		
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hit_1},  duration_ms = 50*duration_multiplier, callback = function(subject) print(1) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hit_2},  duration_ms = 50*duration_multiplier, callback = function(subject) print(2) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hit_3},  duration_ms = 50*duration_multiplier, callback = function(subject) print(3) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hit_4},  duration_ms = 50*duration_multiplier, callback = function(subject) print(4) end },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hit_5},  duration_ms = 50*duration_multiplier, callback = function(subject) print(5) end }
	},
	
	loop_mode = animation.NONE
}

player_melee_hit_cw_animation = create_animation {
	frames = {		
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hit_1cw },  duration_ms = 50*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hit_2cw },  duration_ms = 50*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hit_3cw },  duration_ms = 50*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hit_4cw },  duration_ms = 50*duration_multiplier },
		{ model = { size_multiplier = npc_size_multiplier, image = images.player_hit_5cw },  duration_ms = 50*duration_multiplier }
	},
	
	loop_mode = animation.NONE
}

--enemy_shotgun_animation = create_animation {
--	frames = {
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_shotgun_1},  duration_ms = 40*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_shotgun_2},  duration_ms = 40*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_shotgun_3},  duration_ms = 40*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_shotgun_4},  duration_ms = 40*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_shotgun_5},  duration_ms = 40*duration_multiplier }
--	},
--	
--	loop_mode = animation.INVERSE
--}
--
--enemy_shotgun_shot_animation = create_animation {
--	frames = {
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_shotgun_shot_1 },  duration_ms = 1*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_shotgun_shot_2 },  duration_ms = 1*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_shotgun_shot_3 },  duration_ms = 1*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_shotgun_shot_4 },  duration_ms = 1*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_shotgun_shot_5 },  duration_ms = 15*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_shotgun_shot_4 },  duration_ms = 15*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_shotgun_shot_3 },  duration_ms = 15*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_shotgun_shot_2 },  duration_ms = 15*duration_multiplier },
--		{ model = { size_multiplier = npc_size_multiplier, image = images.enemy_shotgun_shot_1 },  duration_ms = 15*duration_multiplier }
--	},
--	
--	loop_mode = animation.NONE
--}



--enemy_animation_body_shotgun_set = create_animation_set {
--	animations = { 
--		{ event = animation_events.MOVE_CW, animation_response = enemy_shotgun_animation },
--		{ event = animation_events.MOVE_CCW, animation_response = enemy_shotgun_animation },
--		{ event = animation_events.SHOT, animation_response = enemy_shotgun_shot_animation },
--		{ event = animation_events.SWING_CW, animation_response = enemy_shotgun_shot_animation },
--		{ event = animation_events.SWING_CCW, animation_response = enemy_shotgun_shot_animation }
--	}
--}
--
--enemy_animation_body_set = create_animation_set {
--	animations = { 
--		{ event = animation_events.MOVE_CW, animation_response = enemy_animation },
--		{ event = animation_events.MOVE_CCW, animation_response = enemy_animation }
--	}
--}

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

--enemy_animation_legs_set = create_animation_set {
--	animations = { 
--		{ event = animation_events.MOVE, animation_response = enemy_legs_animation }
--	}
--}