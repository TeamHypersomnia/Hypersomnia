background_sprite = create_sprite {
	image = images.background,
	size = vec2(2000, 1000)
}

crosshair_sprite = create_sprite {
	image = images.crosshair
}

crate_sprite = create_sprite {
	image = images.crate,
	size = vec2(100, 100)
}

metal_sprite = create_sprite {
	image = images.metal,
	size = vec2(170, 170)
}

corpse_sprite = create_sprite {
	image = images.dead_front
}

crate_piece_poly = create_polygon {
	{ pos = vec2(0, 0), image = images.crate, texcoord = vec2(0, 0), color = rgba(255, 255, 255, 255) },
	{ pos = vec2(180, 0), image = images.crate, texcoord = vec2(1, 0), color = rgba(255, 255, 255, 255) },
	{ pos = vec2(80, 40), image = images.crate, texcoord = vec2(1, 1), color = rgba(255, 255, 255, 255) },
	{ pos = vec2(0, 80), image = images.crate, texcoord = vec2(0, 1), color = rgba(255, 255, 255, 255) }
}


crate_archetype = {
	transform = {
		pos = vec2(0, 0), rotation = 0
	},
	
	render = {
		model = crate_sprite,
		layer = render_layers.OBJECTS
	},
	
	particle_emitter = {
		available_particle_effects = wood_effects
	},
	
	physics = {
		body_type = Box2D.b2_dynamicBody,
		
		body_info = {
			filter = filter_objects,
			shape_type = physics_info.RECT,
			rect_size = crate_sprite.size,
			
			linear_damping = 5,
			angular_damping = 5,
			fixed_rotation = false,
			density = 0.1,
			sensor = false
		}
	}
}

crate_piece_archetype = archetyped(crate_archetype, {
	render = {
		model = crate_piece_poly
	},
	
	physics = {
		body_info = {
			shape_type = physics_info.POLYGON,
			vertices = crate_piece_poly
		}
	}
})

my_crate_piece = create_entity (archetyped(crate_piece_archetype, {
	transform = {
		pos = vec2(0, 300)
	}
}))

bg = create_entity {
	render = {
		model = background_sprite,
		layer = render_layers.GROUND
	},
	
	transform = {
		pos	= vec2(0, 300),
		rotation = 0
	}
}

bullet_sprite = create_sprite {
	image = images.bullet,
	size_multiplier = vec2(1.5, 0.3)
}

assault_rifle = {
	bullets_once = 1,
	bullet_distance_offset = 120,
	bullet_damage = minmax(80, 100),
	bullet_speed = 7000,
	bullet_render = { model = bullet_sprite, layer = render_layers.BULLETS },
	is_automatic = true,
	max_rounds = 30,
	shooting_interval_ms = 70,
	spread_degrees = 2,
	velocity_variation = 2500,
	shake_radius = 9.5,
	shake_spread_degrees = 45,
	
	bullet_body = {
		filter = filter_bullets,
		shape_type = physics_info.RECT,
		rect_size = bullet_sprite.size,
		fixed_rotation = true,
		density = 0.1
	},
	
	max_bullet_distance = 5000,
	current_rounds = 1000000
}


shotgun = archetyped(assault_rifle, {
	bullets_once = 12,
	bullet_damage = minmax(80, 100),
	bullet_speed = 8000,
	is_automatic = false,
	shooting_interval_ms = 500,
	spread_degrees = 10,
	velocity_variation = 3500
})

my_npc_archetype = {
	body = {
		transform = { 
			pos = vec2(0, 0),
			rotation = 0
		},
		
		render = {
			layer = render_layers.PLAYERS,
			model = nil
		},
		
		animate = {
			available_animations = npc_animation_body_set
		},
		
		health = {
			hp = 1000,
			max_hp = 1000,
			dead = false,
			corpse_render = { model = corpse_sprite, layer = render_layers.ON_GROUND },
			should_disappear = false,
			dead_lifetime_ms = 0,
			
			corpse_body = {
				filter = filter_corpses,
				shape_type = physics_info.RECT,
				rect_size = corpse_sprite.size,
				fixed_rotation = false,
				linear_damping = 20,
				angular_damping = 3
			}
		},
		
		physics = {
			body_type = Box2D.b2_dynamicBody,
			
			body_info = {
				filter = filter_characters,
				shape_type = physics_info.RECT,
				rect_size = images.player_1.size,
				
				linear_damping = 5,
				angular_damping = 5,
				fixed_rotation = false,
				density = 0.1
			},
		},
		
		movement = {
			acceleration = vec2(15000, 15000),
			
			max_speed = 15000,
			
			receivers = {
				{ target = "body", stop_at_zero_movement = false }, 
				{ target = "legs", stop_at_zero_movement = true  }
			}
		},
		
		particle_emitter = {
			available_particle_effects = npc_effects
		},
		
		children = {
			"legs"
		}
	},
	
	legs = {
		transform = { 
			pos = vec2(0, 0),
			rotation = 0
		},
		
		render = {
			layer = render_layers.LEGS,
			model = nil
		},
		
		chase = {
			target = "body"
		},
		
		lookat = {
			target = "body",
			look_mode = lookat_component.VELOCITY
		},
		
		animate = {
			available_animations = npc_animation_legs_set
		}
	}
}

player = create_entity_group (archetyped(my_npc_archetype, {
	body = {
		animate = {
			available_animations = npc_animation_body_shotgun_set
		},
		
		gun = assault_rifle,
		
		physics = {
		
			body_info = {
				fixed_rotation = true,
				angular_damping = 5,
				linear_damping = 13
			}
		},
		
		input = {
			intent_message.MOVE_FORWARD,
			intent_message.MOVE_BACKWARD,
			intent_message.MOVE_LEFT,
			intent_message.MOVE_RIGHT,
			intent_message.SHOOT
		},
		
		lookat = {
			target = "crosshair",
			look_mode = lookat_component.POSITION
		},
		
		ai = {
			visibility_square_side = 2000
		}
	},
	
	crosshair = { 
		transform = {
			pos = vec2(0, 0),
			rotation = 0
		},
		
		render = {
			layer = render_layers.GUI_OBJECTS,
			model = crosshair_sprite
		},
		
		crosshair = {
			sensitivity = 5.5
		},
		
		chase = {
			target = "body",
			relative = true
		},
		
		input = {
			intent_message.AIM
		}
	}
}))

my_scriptable_info = create_scriptable_info {
	scripted_events = {
		[scriptable_component.COLLISION_MESSAGE] 	= 	
		function(message) 
			print ("calling COLLISION_MESSAGE at point X:" .. message.point.x .. " Y: " .. message.point.y) 
			message.collider.physics.body:ApplyForce(b2Vec2(-message.impact_velocity.x*100, -message.impact_velocity.y*100), message.collider.physics.body:GetWorldCenter())
			return true 
			
		end,
		
		[scriptable_component.DAMAGE_MESSAGE] 		= 	
		function(message) 
			message.amount = 10
			print ("calling DAMAGE_MESSAGE with damage of amount " .. message.amount)
			--player.body.physics.body:ApplyForce(b2Vec2(message.impact_velocity.x, message.impact_velocity.y), player.body.physics.body:GetWorldCenter())
			return true
		end
	}
}

npcs = {}
crates = {}
for i = 1, 0 do
	crates[i] = create_entity (archetyped(crate_archetype, {
		transform = {
			pos = vec2(400, i*200-600),
			rotation = 0
		}
	}))
	
	npcs[i] = create_entity_group (archetyped(my_npc_archetype, {
		body = {
			transform = {
				pos = vec2(-550, i*200-600),
				rotation = 0
			},
			
			scriptable = {
				available_scripts = my_scriptable_info
			}
		}
	}))
	
	create_entity (archetyped(crate_archetype, {
		transform = {
			pos = vec2(-400, i*200-600),
			rotation = 0
		},
		
		render = {
			model = metal_sprite
		},
		
		particle_emitter = {
			available_particle_effects = metal_effects
		},
		
		physics = {
			body_info = {
				rect_size = metal_sprite.size,
				density = 0.5
			}
		}
	}))
end

-- msg = create(animate_message, {
-- 	set_animation = nil,
-- 	animation_type = animation_events.SHOT,
-- 	preserve_state_if_animation_changes = false,
-- 	message_type = animate_message.START,
-- 	change_animation = true,
-- 	change_speed = true,
-- 	speed_factor = 0.05,
-- 	animation_priority = 0
-- })
-- 
-- msg.subject = player
-- world:post_message(msg)
-- 
-- msg.subject = npc_1
-- world:post_message(msg)

main_context = create_input_context {
	intents = { 
		[mouse.raw_motion] 		= intent_message.AIM,
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

camera_archetype = {
	transform = {
		pos = vec2(),
		rotation = 0
	},

	camera = {
		enabled = true,
		
		layer = 0, -- 0 = topmost
		mask = render_component.WORLD,
		
		enable_smoothing = true,
		smoothing_average_factor = 0.5,
		averages_per_sec = 20,
		
		crosshair = nil,
		player = nil,
	
		orbit_mode = camera_component.LOOK,
		max_look_expand = vec2(config_table.resolution_w/2, config_table.resolution_h/2),
		angled_look_length = 100
	},
	
	chase = {
		relative = false,
		offset = vec2(config_table.resolution_w/(-2), config_table.resolution_h/(-2))
	}
}

world_camera = create_entity (archetyped(camera_archetype, {
	transform = {
		pos = vec2(),
		rotation = 0
	},

	camera = {
		screen_rect = rect_xywh(0, 0, config_table.resolution_w, config_table.resolution_h),
		ortho = rect_ltrb(0, 0, config_table.resolution_w, config_table.resolution_h),
	
		crosshair = player.crosshair,
		player = player.body,
	},
	
	input = {
		intent_message.SWITCH_LOOK
	},
	
	chase = {
		target = player.body
	}
}))

--npc_camera = create_entity (archetyped(camera_archetype, {
--	transform = {
--		pos = vec2(),
--		rotation = 0
--	},
--
--	camera = {
--		screen_rect = rect_xywh(400, 0, 400, 800),
--		ortho = rect_ltrb(400, 0, 800, 800)
--	},
--	
--	chase = {
--		target = npc_1.body,
--		offset = vec2(config_table.resolution_w/(-1.5), config_table.resolution_h/(-2))
--	}
--}))

player.body.gun.target_camera_to_shake:set(world_camera)


my_scriptable_info:at(scriptable_component.COLLISION_MESSAGE)(1)
my_scriptable_info:at(scriptable_component.DAMAGE_MESSAGE)(2)

--script_only = create_entity {
--	scriptable = {
--		available_scripts = my_scriptable_info
--	}
--}

