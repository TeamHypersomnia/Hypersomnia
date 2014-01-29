-- PARTICLES --

big_wood_piece = {
	angular_damping = 600,
	linear_damping = 5000,
	should_disappear = false
}

small_wood_piece = {
	angular_damping = 600,
	linear_damping = 3000,
	should_disappear = false
}

blood_piece = {
	angular_damping = 1000,
	linear_damping = 80500,
	should_disappear = false,
}



blood_templates = {
	archetyped(blood_piece, { model = { image = images.blood_1 } } ),
	archetyped(blood_piece, { model = { image = images.blood_2 } } ),
	archetyped(blood_piece, { model = { image = images.blood_3 } } ),
	archetyped(blood_piece, { model = { image = images.blood_4 } } ),
	archetyped(blood_piece, { model = { image = images.blood_5 } } )
}

big_wood_templates = {
	archetyped(big_wood_piece, { model = { image = images.piece_1 } }),
	archetyped(big_wood_piece, { model = { image = images.piece_2 } }),
	archetyped(big_wood_piece, { model = { image = images.piece_3 } }),
	archetyped(big_wood_piece, { model = { image = images.piece_4 } })
}

small_wood_templates = {
	archetyped(small_wood_piece, { model = { image = images.piece_5 } }),
	archetyped(small_wood_piece, { model = { image = images.piece_6 } }),
	archetyped(small_wood_piece, { model = { image = images.piece_7 } }),
	archetyped(small_wood_piece, { model = { image = images.piece_8 } })
}

wall_piece = {
	angular_damping = 600,
	linear_damping = 5000,
	should_disappear = false,
	model = { color = rgba(255, 255, 255, 182)  }
}

wall_templates = {
	archetyped(wall_piece, { model = { image = images.wall_piece_1 } }),
	archetyped(wall_piece, { model = { image = images.wall_piece_2 } }),
	archetyped(wall_piece, { model = { image = images.wall_piece_3 } }),
	archetyped(wall_piece, { model = { image = images.wall_piece_4 } }),
	archetyped(wall_piece, { model = { image = images.wall_piece_5 } }),
	archetyped(wall_piece, { model = { image = images.wall_piece_6 } }),
	archetyped(wall_piece, { model = { image = images.wall_piece_7 } }),
	archetyped(wall_piece, { model = { image = images.wall_piece_8 } }),
	archetyped(wall_piece, { model = { image = images.wall_piece_9 } }),
	archetyped(wall_piece, { model = { image = images.wall_piece_10 } })
}

barrel_explosion_template = {
	angular_damping = 0,
	linear_damping = 55000,
	should_disappear = true,
	model = { image = images.bullet, size_multiplier = vec2(4, 4) },
}

barrel_smoke_template = {
	angular_damping = 0,
	linear_damping = 1000,
	should_disappear = true,
	model = { image = images.smoke_particle, color = rgba(255, 255, 255, 6), size_multiplier = vec2(0.2, 0.2) }
	--acc = vec2(0, -300)
}

dust_template = archetyped(small_wood_piece, {
	should_disappear = true,
	model = { image = images.piece_8 }
})

-- EMISSIONS --

wall_parts = {
	spread_degrees = 45,
	type = emission.BURST,
	initial_rotation_variation = 180,
	
	particle_render_template = { 
		layer = render_layers.ON_GROUND
	},
	
	particles_per_burst = minmax_u(3, 6),
	velocity = minmax(10, 700),
	angular_velocity = minmax(10, 100),
	size_multiplier = minmax(0.3, 1),
	
	particle_templates = wall_templates
}

wood_emission_archetype = {
	spread_degrees = 45,
	type = emission.BURST,
	initial_rotation_variation = 180,
	
	particle_render_template = { 
		layer = render_layers.ON_GROUND
	}
}

wood_parts_big = archetyped(wood_emission_archetype, {
	particles_per_burst = minmax_u(1, 1),
	velocity = minmax(500, 1000),
	angular_velocity = minmax(100, 220),
	size_multiplier = minmax(0.1, 0.5),
	
	particle_templates = big_wood_templates
})

wood_parts_small = archetyped(wood_emission_archetype, {
	particles_per_burst = minmax_u(1, 4),
	velocity = minmax(100, 1500),
	angular_velocity = minmax(100, 220),
	size_multiplier = minmax(0.1, 0.5),
	
	particle_templates = small_wood_templates
})

wood_dust = archetyped(wood_emission_archetype, {
	particles_per_burst = minmax_u(5, 55),
	velocity = minmax(200, 4000),
	angular_velocity = minmax(540, 1220),
	particle_lifetime_ms = minmax(0, 350),
	particle_templates = { dust_template }
})

sparkles = {
	spread_degrees = 15,
	particles_per_burst = minmax_u(0, 50),
	type = emission.BURST,
	velocity = minmax(1000, 10000),
	angular_velocity = minmax(0, 0),
	particle_lifetime_ms = minmax(20, 40),
	particle_templates = {
		archetyped(dust_template, { 
			model = {
				image = images.bullet,
				size_multiplier = vec2(0.1, 0.05),
				color = rgba(255, 255, 255, 120)
			}
		})
	},
	
	size_multiplier = minmax(0.001, 5),
		
	particle_render_template = { 
		layer = render_layers.EFFECTS
	}
}

barrel_explosion = {
	spread_degrees = 15.5,
	particles_per_burst = minmax_u(10, 50),
	type = emission.BURST,
	velocity = minmax(1000, 10000),
	angular_velocity = minmax(0, 0),
	particle_lifetime_ms = minmax(0, 20),
	particle_templates = {
		barrel_explosion_template
	},
	
	size_multiplier = minmax(0.8, 1.2),
		
	particle_render_template = { 
		layer = render_layers.EFFECTS
	},
	initial_rotation_variation = 20
}

barrel_smoke_1 = {
	spread_degrees = 0.0,
	particles_per_sec = minmax(100, 250),
	stream_duration_ms = minmax(100, 600),
	type = emission.STREAM,
	velocity = minmax(300, 500),
	particle_lifetime_ms = minmax(10, 1500),
	
	particle_templates = {
		barrel_smoke_template
	},
	
	size_multiplier = minmax(0.3, 5),
	initial_rotation_variation = 180,
	
	particle_render_template = { 
		layer = render_layers.EFFECTS
	},
	
	--swing_spread = minmax(0, 90),
	--swings_per_sec = minmax(2, 10)
	
	min_swing_spread = minmax(2, 5),
	min_swings_per_sec = minmax(0.5, 1),
	max_swing_spread = minmax(6, 12),
	max_swings_per_sec = minmax(1.5, 2),
	
	swing_spread = minmax(5, 12),
	swings_per_sec = minmax(2, 2),
	swing_spread_change_rate = minmax(1, 2),
	angular_offset = minmax(0, 0),
	
	fade_when_ms_remaining = minmax(10, 50)
	--swing_speed_change_rate = minmax(0.05, 0.06)
}

barrel_smoke_2 = archetyped(barrel_smoke_1, {
	spread_degrees = 0,
	stream_duration_ms = minmax(1000, 6000),
	particles_per_sec = minmax(1000, 1000),
	velocity = minmax(180, 180),
	--particle_lifetime_ms = minmax(10, 3000),
	
	size_multiplier = minmax(0.5, 0.7),
	
	particle_templates = {
		{ linear_damping = 5 }
	},
	
	fade_when_ms_remaining = minmax(3000, 5000)
	--swing_spread = minmax(10, 40),
	--swings_per_sec = minmax(0.005, 0.005)
})

bullet_impact_smoke_1 = archetyped(barrel_smoke_1, {
	particles_per_sec = minmax(190, 290),
	stream_duration_ms = minmax(100, 600),
})

bullet_impact_smoke_2 = archetyped(barrel_smoke_2, {
	particles_per_sec = minmax(190, 290),
	stream_duration_ms = minmax(1000, 6000),
})

bullet_shell_smoke = archetyped(barrel_smoke_2, {
	particles_per_sec = minmax(100, 100),
	stream_duration_ms = minmax(1000, 2000),
	
	size_multiplier = minmax(0.4, 0.4),
	
	particle_templates = {
		{ model = { color = rgba(255, 255, 255, 20) } }
	}
})

blood_shower = {
	spread_degrees = 10,
	angular_offset = minmax(0, 180),
	particles_per_burst = minmax_u(35, 45),
	type = emission.BURST;
	velocity = minmax(1, 3000),
	angular_velocity = minmax(0, 0),
	
	particle_templates = blood_templates,
	
	size_multiplier = minmax(0.25, 0.65),
	initial_rotation_variation = 180,
	
	particle_render_template = { 
		layer = render_layers.ON_GROUND
	}
}

blood_droplets = {
	spread_degrees = 2.5,
	angular_offset = minmax(0, 10),
	particles_per_burst = minmax_u(600, 1000),
	type = emission.BURST;
	velocity = minmax(1, 5000),
	angular_velocity = minmax(0, 0),
	
	particle_templates = archetyped(blood_templates, {
		{ should_disappear = true },
		{ should_disappear = true },
		{ should_disappear = true },
		{ should_disappear = true },
		{ should_disappear = true }
	}),
	
	size_multiplier = minmax(1, 1),
	initial_rotation_variation = 180,
	
	particle_render_template = { 
		layer = render_layers.ON_GROUND
	},
	
	particle_lifetime_ms = minmax(50, 150)
}

blood_pool = {
	spread_degrees = 1.5,
	particles_per_sec = minmax(350, 1050),
	stream_duration_ms = minmax(100, 200),
	type = emission.STREAM,
	velocity = minmax(0.1, 50),
	angular_velocity = minmax(0, 0),
	
	particle_templates = archetyped(blood_templates, {
		{ linear_damping = 1000 },
		{ linear_damping = 1000 },
		{ linear_damping = 1000 },
		{ linear_damping = 1000 },
		{ linear_damping = 1000 }
	}),
	
	size_multiplier = minmax(1, 1),
	initial_rotation_variation = 180,
	
	particle_render_template = { 
		layer = render_layers.ON_GROUND
	},
	
	min_swing_spread = minmax(2, 5),
	min_swings_per_sec = minmax(0.5, 1),
	max_swing_spread = minmax(6, 12),
	max_swings_per_sec = minmax(1.5, 2),
	
	swing_spread = minmax(5, 12),
	swings_per_sec = minmax(2, 2),
	swing_spread_change_rate = minmax(1, 2),
}

blood_under_corpse = {
	spread_degrees = 180,
	particles_per_sec = minmax(400, 400),
	stream_duration_ms = minmax(3000, 3000),
	type = emission.STREAM,
	velocity = minmax(0, 6),
	angular_velocity = minmax(0, 0),
	
	particle_templates = archetyped(blood_templates, {
		{ linear_damping = 0.5 },
		{ linear_damping = 0.5 },
		{ linear_damping = 0.5 },
		{ linear_damping = 0.5 },
		{ linear_damping = 0.5 }
	}),
	
	size_multiplier = minmax(1.5, 2),
	initial_rotation_variation = 180,
	
	particle_render_template = { 
		layer = render_layers.UNDER_CORPSES
	}
}


-- EFFECTS --
wood_effect = {
	bullet_impact_smoke_1,
	bullet_impact_smoke_2,
	wood_parts_big,
	wood_parts_small,
	wood_dust
}

metal_effect = {
	wall_parts,
	bullet_impact_smoke_1,
	bullet_impact_smoke_2,
	archetyped(sparkles, {
		velocity = minmax(1000, 2000),
		particle_lifetime_ms = minmax(1, 300),
		
		angular_offset = minmax(-20, 20),
		
		particle_templates = {
			{ model = { color = rgba(255, 255, 255, 255) } 
			}
		}
	})
}

gunshot_effect = {
	barrel_smoke_1,
	barrel_smoke_2,
	barrel_explosion,
	archetyped(sparkles, {
		size_multiplier = minmax(0.1, 35),
		angular_offset = minmax(-15, 15),
		particles_per_burst = minmax_u(10, 150),
		velocity = minmax(1000, 3400),
		initial_rotation_variation = 40
	})
}

blood_effect = {
	blood_shower,
	blood_shower
	--blood_pool,
	--blood_droplets
}

blood_under_corpse_effect = create_particle_effect {
	blood_under_corpse
}

bullet_shell_smoke_effect = create_particle_effect {
	bullet_shell_smoke
} 

-- EFFECT SETS --
wood_effects = create_particle_emitter_info {
	effects_subscribtion = {
		[particle_burst_message.BULLET_IMPACT] = wood_effect
	}
}


metal_effects = create_particle_emitter_info {
	effects_subscribtion = {
		[particle_burst_message.BULLET_IMPACT] = metal_effect
	}
}

npc_effects = create_particle_emitter_info {
	effects_subscribtion = {
		[particle_burst_message.BULLET_IMPACT] = blood_effect,
		[particle_burst_message.WEAPON_SHOT] = gunshot_effect
	}
}

-- ONLY FOR COMMANDS.LUA!!! --
-- ONLY FOR COMMANDS.LUA!!! --
-- ONLY FOR COMMANDS.LUA!!! --
-- have to declare these effects here as otherwise they would get garbage collected on every call to commands.lua!!!--
cached_wood_effect = create_particle_effect(wood_effect)