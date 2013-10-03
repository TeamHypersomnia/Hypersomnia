-- PARTICLES --

big_wood_piece = {
	angular_damping = 1200,
	linear_damping = 10000,
	should_disappear = false
}

small_wood_piece = {
	angular_damping = 1200,
	linear_damping = 7000,
	should_disappear = false
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

dust_template = archetyped(small_wood_piece, {
	should_disappear = true,
	max_lifetime_ms = 200,
	model = { image = images.piece_8 }
})

-- EMISSIONS --

wood_emission_archetype = {
	spread_degrees = 45,
	type = emission.BURST,
	angular_offset = 0,
	initial_rotation_variation = 180,
	
	particle_render_template = { 
		layer = render_layers.ON_GROUND, 
		mask = render_component.WORLD 
	}
}

wood_parts_big = archetyped(wood_emission_archetype, {
	particles_per_burst = minmax_u(1, 1),
	velocity = minmax(2000, 3000),
	angular_velocity = minmax(100, 220),
	size_multiplier = minmax(0.1, 1.0),
	
	particle_templates = big_wood_templates
})

wood_parts_small = archetyped(wood_emission_archetype, {
	particles_per_burst = minmax_u(2, 4),
	velocity = minmax(200, 3000),
	angular_velocity = minmax(100, 220),
	size_multiplier = minmax(0.1, 1.0),
	
	particle_templates = small_wood_templates
})

wood_dust = archetyped(wood_emission_archetype, {
	particles_per_burst = minmax_u(5, 55),
	velocity = minmax(200, 4000),
	angular_velocity = minmax(540, 1220),
	size_multiplier = minmax(0.1, 0.5),
	
	particle_lifetime_ms = minmax(0, 350),
	particle_templates = { dust_template }
})

-- EFFECTS --
wood_effect = {
	emissions = {
		wood_parts_big,
		wood_parts_small,
		wood_dust
	}
}

-- EFFECT SETS --
wood_effects = create_particle_emitter_info {
	effects_subscribtion = {
		[particle_burst_message.BULLET_IMPACT] = wood_effect
	}
}