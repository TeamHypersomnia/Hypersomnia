-- PARTICLES --

function create_particle_effects(scene)
	local sprites = scene.sprite_library

	scene.particles = {}
	local particles = scene.particles
	
	particles.blood_piece = {
		angular_damping = 1000,
		linear_damping = 80500,
		should_disappear = true,
		alpha_levels = 2
	}
	
	particles.blood_templates = {
		override(particles.blood_piece, { model = { image = sprites.blood["1"] } } ),
		override(particles.blood_piece, { model = { image = sprites.blood["2"] } } ),
		override(particles.blood_piece, { model = { image = sprites.blood["3"] } } ),
		override(particles.blood_piece, { model = { image = sprites.blood["4"] } } ),
		override(particles.blood_piece, { model = { image = sprites.blood["5"] } } )
	}
	
	particles.wall_piece = {
		angular_damping = 600,
		linear_damping = 5000,
		should_disappear = true,
		model = { color = rgba(0, 0, 0, 255)  },
		alpha_levels = 1
	}
	
	particles.wall_templates = {
		override(particles.wall_piece, { model = { image = sprites.wall.piece["1"] } }),
		override(particles.wall_piece, { model = { image = sprites.wall.piece["2"] } }),
		override(particles.wall_piece, { model = { image = sprites.wall.piece["3"] } }),
		override(particles.wall_piece, { model = { image = sprites.wall.piece["4"] } }),
		override(particles.wall_piece, { model = { image = sprites.wall.piece["5"] , color = rgba(100, 100, 100, 255)} }),
		override(particles.wall_piece, { model = { image = sprites.wall.piece["6"] , color = rgba(80, 80, 80, 255) } }),
		override(particles.wall_piece, { model = { image = sprites.wall.piece["7"] , color = rgba(80, 80, 80, 255) } }),
		override(particles.wall_piece, { model = { image = sprites.wall.piece["8"] , color = rgba(80, 80, 80, 255) } }),
		override(particles.wall_piece, { model = { image = sprites.wall.piece["9"] , color = rgba(80, 80, 80, 255) } }),
		override(particles.wall_piece, { model = { image = sprites.wall.piece["10"], color = rgba(80, 80, 80, 255)  } })
	}
	
	particles.barrel_explosion_template = {
		angular_damping = 0,
		linear_damping = 5000,
		should_disappear = true,
		model = { image = sprites.barrel1, size_multiplier = vec2(1, 1), color = rgba(0, 254, 254, 255) },
		alpha_levels = 1
	}
	
	particles.barrel_smoke_template = {
		angular_damping = 0,
		linear_damping = 10,
		should_disappear = true,
		model = { image = sprites.smoke1, color = rgba(0, 254, 254, 126), size_multiplier = vec2(0.4, 0.4) },
		--alpha_levels = 255,
		--ignore_rotation = true
		--acc = vec2(0, -300)
	}
	
	-- EMISSIONS --
	
	particles.wall_parts = {
		spread_degrees = minmax(65, 65),
		type = emission.BURST,
		particle_lifetime_ms = minmax(20000, 40000),
		initial_rotation_variation = 180,
		
		particle_render_template = { 
			layer = render_layers.ON_GROUND
		},
		
		particles_per_burst = minmax_u(0, 2),
		velocity = minmax(10, 1200),
		angular_velocity = minmax(10, 100),
		size_multiplier = minmax(0.1, 1.4),
		
		particle_templates = particles.wall_templates
	}
	
	--particles.sparkles = {
	--	spread_degrees = 15,
	--	particles_per_burst = minmax_u(0, 50),
	--	type = emission.BURST,
	--	velocity = minmax(1000, 10000),
	--	angular_velocity = minmax(0, 0),
	--	particle_lifetime_ms = minmax(20, 40),
	--	particle_templates = {
	--		{ 
	--			should_disappear = true,
	--			model = {
	--				image = sprites.bullet,
	--				size_multiplier = vec2(0.1, 0.05),
	--				color = rgba(255, 255, 255, 120)
	--			}
	--		}
	--	},
	--	
	--	size_multiplier = minmax(0.001, 5),
	--		
	--	particle_render_template = { 
	--		layer = render_layers.EFFECTS
	--	}
	--}
	
	particles.barrel_explosion = {
		spread_degrees = minmax(10, 15),
		particles_per_burst = minmax_u(30, 120),
		type = emission.BURST,
		velocity = minmax(10, 800),
		angular_velocity = minmax(0, 0),
		particle_lifetime_ms = minmax(1, 120),
		particle_templates = {
			particles.barrel_explosion_template,
			override(particles.barrel_explosion_template, {
				model = { image = sprites.barrel2 }
			}),
			override(particles.barrel_explosion_template, {
				model = { image = sprites.barrel3 }
			}),
			override(particles.barrel_explosion_template, {
				model = { image = sprites.barrel4 }
			}),
			override(particles.barrel_explosion_template, {
				model = { image = sprites.barrel5 }
			})
		},
		
		size_multiplier = minmax(0.5, 1),
			
		particle_render_template = { 
			layer = render_layers.EFFECTS
		},
		
		initial_rotation_variation = 0
	}
	
	particles.sparkles = override (particles.barrel_explosion, {
		spread_degrees = minmax(180, 180),
		velocity = minmax(10, 800),
		particle_lifetime_ms = minmax(10, 130),
		size_multiplier = minmax(0.4, 0.7),
		initial_rotation_variation = 0,


		particle_templates = {
			override(particles.barrel_explosion_template, {
				model = { image = sprites.barrel1 }, linear_damping = 7400
			}),
			override(particles.barrel_explosion_template, {
				model = { image = sprites.barrel2 }, linear_damping = 7400
			}),
			override(particles.barrel_explosion_template, {
				model = { image = sprites.barrel3 }, linear_damping = 7400
			}),
			override(particles.barrel_explosion_template, {
				model = { image = sprites.barrel4 }, linear_damping = 7400
			}),
			override(particles.barrel_explosion_template, {
				model = { image = sprites.barrel5 }, linear_damping = 7400
			})
		},
	})
	
	particles.barrel_smoke_1 = {
		spread_degrees = minmax(0, 5),
		particles_per_sec = minmax(5, 560),
		stream_duration_ms = minmax(100, 600),
		type = emission.STREAM,
		velocity = minmax(20, 30),
		particle_lifetime_ms = minmax(4000, 4000),
		angular_velocity = minmax(0.4, 0.8),
		
		particle_templates = {
			particles.barrel_smoke_template,
			override(particles.barrel_smoke_template, {
				model = { image = sprites.smoke2 }
			})
			,
			override(particles.barrel_smoke_template, {
				model = { image = sprites.smoke3 }
			}),
			override(particles.barrel_smoke_template, {
				model = { image = sprites.smoke4 }
			}),
			override(particles.barrel_smoke_template, {
				model = { image = sprites.smoke5 }
			}),
			override(particles.barrel_smoke_template, {
				model = { image = sprites.smoke6 }
			})
		},
		
		size_multiplier = minmax(0.3, 0.4),
		initial_rotation_variation = 180,
		
		particle_render_template = { 
			layer = render_layers.SMOKES
		},
		
		--swing_spread = minmax(0, 90),
		--swings_per_sec = minmax(2, 10)
		
		min_swing_spread = minmax(2, 5),
		min_swings_per_sec = minmax(0.5, 1),
		max_swing_spread = minmax(6, 12),
		max_swings_per_sec = minmax(1.5, 4),
		
		swing_spread = minmax(5, 52),
		swings_per_sec = minmax(2, 8),
		swing_spread_change_rate = minmax(1, 4),
		angular_offset = minmax(0, 0),
		
		fade_when_ms_remaining = minmax(10, 50)
		--swing_speed_change_rate = minmax(0.05, 0.06)
	}
	
	particles.barrel_smoke_2 = override(particles.barrel_smoke_1, {
		spread_degrees = minmax(0, 5),
		stream_duration_ms = minmax(5000, 6000),
		particles_per_sec = minmax(30, 400),
		velocity = minmax(170, 180),
		--particle_lifetime_ms = minmax(5000, 6000),
		--particle_lifetime_ms = minmax(10, 3000),
		
		size_multiplier = minmax(0.3, 0.5),
		
		fade_when_ms_remaining = minmax(100, 200)
		--swing_spread = minmax(10, 40),
		--swings_per_sec = minmax(0.005, 0.005)
	})

	particles.born_smoke_1 = override(particles.barrel_smoke_1, {
		velocity = minmax(20, 150),
		spread_degrees = minmax(0, 0),
		particle_lifetime_ms = minmax(1000, 3000),
		particles_per_sec = minmax(300, 400),


		min_swing_spread = minmax(0, 0),
		min_swings_per_sec = minmax(0, 0),
		max_swing_spread = minmax(0, 0),
		max_swings_per_sec = minmax(0, 0),

		swing_spread = minmax(20, 20),
		swings_per_sec = minmax(8, 8),
		swing_spread_change_rate = minmax(0, 0)
	})
	
	particles.bullet_impact_smoke_1 = override(particles.barrel_smoke_1, {
		particles_per_sec = minmax(5, 5),
		velocity = minmax(30, 70),
		num_of_particles_to_spawn_initially = minmax(55, 55),
		stream_duration_ms = minmax(3000, 3000),
		spread_degrees = minmax(180, 180),

		angular_velocity = minmax(1.8, 1.8),

		size_multiplier = minmax(0.2, 0.5)
	})	

	particles.blood_impact = override(particles.bullet_impact_smoke_1, {
		particles_per_sec = minmax(5, 5),
		velocity = minmax(30, 70),
		num_of_particles_to_spawn_initially = minmax(55, 55),
		stream_duration_ms = minmax(3000, 3000),
		spread_degrees = minmax(180, 180),
		
		particle_templates = {			
			override(particles.barrel_smoke_template, {
				model = { image = sprites.smoke1, color = rgba(255, 0, 0, 126) }
			}),
			override(particles.barrel_smoke_template, {
				model = { image = sprites.smoke2, color = rgba(255, 0, 0, 126) }
			})
			,
			override(particles.barrel_smoke_template, {
				model = { image = sprites.smoke3, color = rgba(255, 0, 0, 126) }
			}),
			override(particles.barrel_smoke_template, {
				model = { image = sprites.smoke4, color = rgba(255, 0, 0, 126) }
			}),
			override(particles.barrel_smoke_template, {
				model = { image = sprites.smoke5, color = rgba(255, 0, 0, 126) }
			}),
			override(particles.barrel_smoke_template, {
				model = { image = sprites.smoke6, color = rgba(255, 0, 0, 126) }
			})
		},

		angular_velocity = minmax(1.8, 1.8),

		size_multiplier = minmax(0.2, 0.5)
	})
	
	particles.bullet_impact_smoke_2 = override(particles.barrel_smoke_2, {
		particles_per_sec = minmax(10, 20),
		stream_duration_ms = minmax(1000, 6000),
	})
	
	particles.bullet_shell_smoke = override(particles.barrel_smoke_2, {
		particles_per_sec = minmax(100, 100),
		stream_duration_ms = minmax(1000, 2000),
		
		size_multiplier = minmax(1, 2),
		
		particle_templates = {
			{ model = { color = rgba(255, 255, 255, 20) } }
		}
	})
	
	particles.blood_shower = {
		spread_degrees = minmax(180, 180),
		angular_offset = minmax(0, 180),
		particles_per_burst = minmax_u(5, 15),
		type = emission.BURST;
		velocity = minmax(1, 3000),
		angular_velocity = minmax(0, 0),
		
		particle_templates = particles.blood_templates,
		
		size_multiplier = minmax(0.5, 1),
		initial_rotation_variation = 180,
		
		particle_lifetime_ms = minmax(10250, 10250),
		
		particle_render_template = { 
			layer = render_layers.ON_GROUND
		},

		fade_when_ms_remaining = minmax(1, 2)
	}
	
	particles.blood_droplets = {
		spread_degrees = minmax(2.5, 2.5),
		angular_offset = minmax(0, 10),
		particles_per_burst = minmax_u(600, 1000),
		type = emission.BURST;
		velocity = minmax(1, 5000),
		angular_velocity = minmax(0, 0),
		
		particle_templates = override(particles.blood_templates, {
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
	
	particles.blood_pool = {
		spread_degrees = minmax(1.5, 1.5),
		particles_per_sec = minmax(350, 1050),
		stream_duration_ms = minmax(100, 200),
		type = emission.STREAM,
		velocity = minmax(0.1, 50),
		angular_velocity = minmax(0, 0),
		
		particle_templates = override(particles.blood_templates, {
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
	
	particles.blood_under_corpse = {
		spread_degrees = minmax(180, 180),
		particles_per_sec = minmax(400, 400),
		stream_duration_ms = minmax(3000, 3000),
		type = emission.STREAM,
		velocity = minmax(0, 6),
		angular_velocity = minmax(0, 0),
		
		particle_templates = override(particles.blood_templates, {
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
	
	particles.metal_sparkles = {		
		spread_degrees = minmax(15, 15),
		particles_per_burst = minmax_u(0, 50),
		type = emission.BURST,
		velocity = minmax(700, 1100),
		angular_velocity = minmax(0, 0),
		particle_lifetime_ms = minmax(20, 220),
		particle_templates = {
			{ 
				should_disappear = true,
				model = {
					image = sprites.bullet,
					--size_multiplier = vec2(0.1, 0.05),
					color = rgba(255, 255, 255, 120)
				}
			}
		},
		
		size_multiplier = minmax(0.1, 0.45),
			
		particle_render_template = { 
			layer = render_layers.EFFECTS
		}
	}
	

	particles.fire_emission = {
		spread_degrees = minmax(45, 45),
		particles_per_sec = minmax(15, 55),
		stream_duration_ms = minmax(60000000, 60000000),
		type = emission.STREAM,
		velocity = minmax(10, 40),
		particle_lifetime_ms = minmax(1000, 2000),
		angular_velocity = minmax(0.3, 0.6),
		
		particle_templates = {
			override(particles.barrel_smoke_template, {
				model = { image = sprites.smoke2, color = rgba(255, 200, 0, 220) }
			})
			,
			override(particles.barrel_smoke_template, {
				model = { image = sprites.smoke3, color = rgba(255, 200, 0, 220) }
			}),
			override(particles.barrel_smoke_template, {
				model = { image = sprites.smoke4, color = rgba(255, 200, 0, 220) }
			}),
			override(particles.barrel_smoke_template, {
				model = { image = sprites.smoke5, color = rgba(255, 200, 0, 220) }
			}),
			override(particles.barrel_smoke_template, {
				model = { image = sprites.smoke6, color = rgba(255, 200, 0, 220) }
			})
		},
		
		size_multiplier = minmax(0.3, 0.5),
		initial_rotation_variation = 180,
		
		particle_render_template = { 
			layer = render_layers.SMOKES
		},
		
		--swing_spread = minmax(0, 90),
		--swings_per_sec = minmax(2, 10)
		
		fade_when_ms_remaining = minmax(10, 50)
		--swing_speed_change_rate = minmax(0.05, 0.06)
	}

	particles.cyan_fire = override(particles.fire_emission, {
			stream_duration_ms = minmax(3000, 3000),
		particles_per_sec = minmax(55, 55),

		particle_templates = {
			override(particles.barrel_smoke_template, {
				model = { image = sprites.smoke2, color = rgba(0, 254, 254, 240) }
			})
			,
			override(particles.barrel_smoke_template, {
				model = { image = sprites.smoke3, color = rgba(0, 254, 254, 240) }
			}),
			override(particles.barrel_smoke_template, {
				model = { image = sprites.smoke4, color = rgba(0, 254, 254, 240) }
			}),
			override(particles.barrel_smoke_template, {
				model = { image = sprites.smoke5, color = rgba(0, 254, 254, 240) }
			}),
			override(particles.barrel_smoke_template, {
				model = { image = sprites.smoke6, color = rgba(0, 254, 254, 240) }
			})
		},
	})

	particles.metal_effect = {
		--particles.wall_parts,
		particles.bullet_impact_smoke_1,
		particles.cyan_fire,
		--particles.bullet_impact_smoke_2,
		particles.sparkles
	}
	
	particles.gunshot_effect = {
		--particles.barrel_smoke_1,
		particles.bullet_impact_smoke_1,
		particles.barrel_explosion,
		particles.cyan_fire
		--override(particles.sparkles, {
		--	size_multiplier = minmax(0.1, 35),
		--	angular_offset = minmax(-15, 15),
		--	particles_per_burst = minmax_u(10, 150),
		--	velocity = minmax(1000, 3400),
		--	initial_rotation_variation = 40
		--})
	}
	
	particles.blood_effect = {
		particles.blood_shower,
		particles.blood_impact,
		particles.sparkles
		--particles.blood_shower
		--blood_pool,
		--blood_droplets
	}

	particles.fire_effect = create_particle_effect {
		particles.fire_emission
	}

	particles.born_effect = create_particle_effect {
		particles.born_smoke_1
	}

	particles.blood_under_corpse_effect = create_particle_effect {
		particles.blood_under_corpse
	}
	
	particles.bullet_shell_smoke_effect = create_particle_effect {
		particles.bullet_shell_smoke
	} 
	
	particles.metal_response = {
		BULLET_IMPACT = particles.metal_effect
	}
	
	particles.npc_response = {
		BULLET_IMPACT = particles.blood_effect
	}
	
	particles.gun_response = {
		WEAPON_SHOT = particles.gunshot_effect
	}
end