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

barrel_explosion_template = {
	angular_damping = 0,
	linear_damping = 55000,
	should_disappear = true,
	model = { image = images.bullet, size_multiplier = vec2(4, 4) },
}

barrel_smoke_template = {
	angular_damping = 0,
	linear_damping = 500,
	should_disappear = true,
	model = { image = images.smoke_particle, color = rgba(255, 255, 255, 3), size_multiplier = vec2(0.2, 0.2) }
	--acc = vec2(0, -300)
}

dust_template = archetyped(small_wood_piece, {
	should_disappear = true,
	model = { image = images.piece_8 }
})

-- EMISSIONS --

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
	spread_degrees = 0.5,
	particles_per_sec = minmax(190, 290),
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
	swing_spread = minmax(10, 20),
	swings_per_sec = minmax(0.005, 0.005),
	angular_offset = minmax(-10, 10)
}

barrel_smoke_2 = archetyped(barrel_smoke_1, {
	spread_degrees = 14,
	stream_duration_ms = minmax(100, 6000),
	particles_per_sec = minmax(190, 290),
	velocity = minmax(100, 200),
	--particle_lifetime_ms = minmax(10, 3000),
	
	size_multiplier = minmax(0.2, 1),
	
	particle_templates = {
		{ linear_damping = 10 }
	},
	--swing_spread = minmax(10, 40),
	--swings_per_sec = minmax(0.005, 0.005)
})

blood_shower = {
	spread_degrees = 120.5,
	particles_per_burst = minmax_u(5, 10),
	type = emission.BURST;
	velocity = minmax(1, 4000),
	angular_velocity = minmax(0, 1500),
	
	particle_templates = blood_templates,
	
	size_multiplier = minmax(0.2, 0.35),
	initial_rotation_variation = 180,
	
	particle_render_template = { 
		layer = render_layers.ON_GROUND
	}
}

blood_droplets = {
	spread_degrees = 90.5,
	particles_per_burst = minmax_u(10, 500),
	type = emission.BURST;
	velocity = minmax(1, 5000),
	angular_velocity = minmax(1, 5000),
	
	particle_templates = archetyped(blood_templates, {
		{ should_disappear = true },
		{ should_disappear = true },
		{ should_disappear = true },
		{ should_disappear = true },
		{ should_disappear = true }
	}),
	
	size_multiplier = minmax(0.2, 0.35),
	initial_rotation_variation = 180,
	
	particle_render_template = { 
		layer = render_layers.ON_GROUND
	},
	
	particle_lifetime_ms = minmax(200, 500)
}

blood_pool = {
	spread_degrees = 180.5,
	particles_per_sec = minmax(20, 100),
	stream_duration_ms = minmax(100, 300),
	type = emission.STREAM,
	velocity = minmax(1, 6),
	angular_velocity = minmax(0, 10),
	
	particle_templates = archetyped(blood_templates, {
		{ linear_damping = 2 },
		{ linear_damping = 2 },
		{ linear_damping = 2 },
		{ linear_damping = 2 },
		{ linear_damping = 2 }
	}),
	
	size_multiplier = minmax(0.3, 1.0),
	initial_rotation_variation = 180,
	
	particle_render_template = { 
		layer = render_layers.ON_GROUND
	}
}


-- EFFECTS --
wood_effect = {
	barrel_smoke_1,
	barrel_smoke_2,
	wood_parts_big,
	wood_parts_small,
	wood_dust
}

metal_effect = {
	barrel_smoke_1,
	barrel_smoke_2,
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
	--blood_shower,
	--blood_pool,
	blood_droplets
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