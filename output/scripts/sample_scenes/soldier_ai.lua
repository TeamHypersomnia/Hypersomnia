dofile "scripts\\sample_scenes\\camera.lua"

crosshair_sprite = create_sprite {
	image = images.crosshair,
	color = rgba(255, 0, 0, 255)
}

crosshair_blue = create_sprite {
	image = images.crosshair,
	color = rgba(255, 255, 255, 255)
}

blank_white = create_sprite {
	image = images.blank,
	color = rgba(255, 0, 0, 255)
}

blank_red = create_sprite {
	image = images.blank,
	size_multiplier = vec2(70, 10),
	color = rgba(255, 0, 0, 255)
}

blank_green = create_sprite {
	image = images.blank,
	size_multiplier = vec2(7, 7),
	color = rgba(0, 255, 0, 255)
}

blank_blue = create_sprite {
	image = images.blank,
	size_multiplier = vec2(17, 17),
	color = rgba(0, 0, 255, 255)
}

point_archetype = {
	image = images.metal,
	color = rgba(255, 255, 255, 255),
	texcoord = vec2(0, 0)
}

ground_point_archetype = {
	image = images.background,
	color = rgba(255, 255, 255, 255),
	texcoord = vec2(0, 0)
}

corpse_sprite = create_sprite {
	image = images.dead_front
}

dofile "scripts\\sample_scenes\\map.lua"

small_box_archetype = {
	transform = {
		pos = vec2(0, 0), rotation = 0
	},
	
	render = {
		model = blank_red,
		layer = render_layers.OBJECTS
	},
	
	physics = {
		body_type = Box2D.b2_dynamicBody,
		
		body_info = {
			filter = filter_objects,
			shape_type = physics_info.RECT,
			rect_size = blank_red.size,
			
			linear_damping = 5,
			angular_damping = 5,
			fixed_rotation = false,
			density = 0.1,
			friction = 0,
			sensor = false
		}
	}
}

big_box_archetype = (archetyped(small_box_archetype, {
	render = {
		model = blank_blue
	},

	physics = {
		body_info = {
			rect_size = blank_blue.size,
			density = 0.5
		}
	}
}))

bullet_sprite = create_sprite {
	image = images.bullet,
	size_multiplier = vec2(1.0, 0.4)
}

dofile "scripts\\sample_scenes\\steering.lua"
dofile "scripts\\sample_scenes\\weapons.lua"
dofile "scripts\\sample_scenes\\soldier_tree.lua"

npc_damage_handler = create_scriptable_info {
	scripted_events = {	
		[scriptable_component.DAMAGE_MESSAGE] = function(message)
			--if message.subject.scriptable == nil then return false end
	
			local npc_info = get_scripted(message.subject)
			npc_info.health_info.hp = npc_info.health_info.hp - message.amount
			npc_info.last_impact = message.impact_velocity
			
			return false
		end	
	}
}

corpse_archetype = {
	render = {
		layer = render_layers.CORPSES 
	},
	
	transform = {},
	physics = {
		body_type = Box2D.b2_dynamicBody,
		
		body_info = {
			filter = filter_corpses,
			shape_type = physics_info.RECT,
			rect_size = corpse_sprite.size,
			fixed_rotation = true,
			linear_damping = 5,
			angular_damping = 3
		}
	}
}

head_walk_sprite = create_sprite {
	image = images.head_walk
}

head_gun_sprite = create_sprite {
	image = images.head_gun
}

head_shot_sprite = create_sprite {
	image = images.head_shot
}

head_over_sprite = create_sprite {
	image = images.head_over
}

character_archetype = {
	body = {
		transform = { 
			pos = vec2(0, 0),
			rotation = 0
		},
		
		render = {
			layer = render_layers.PLAYERS,
			model = blank_green
		},
		
		animate = { 
			--available_animations = npc_animation_body_shotgun_set
		},
		gun = {}, 
		--assault_rifle_info,
		
		physics = {
			body_type = Box2D.b2_dynamicBody,
			
			body_info = {
				filter = filter_characters,
				shape_type = physics_info.RECT,
				rect_size = blank_green.size,
				
				angular_damping = 5,
				linear_damping = 18,
				
				fixed_rotation = true,
				density = 0.1
			},
		},
		
		visibility = {
			visibility_layers = {
				[visibility_component.DYNAMIC_PATHFINDING] = {
					square_side = 5000,
					color = rgba(0, 255, 255, 120),
					ignore_discontinuities_shorter_than = -1,
					filter = filter_pathfinding_visibility
				}
				--,
				--
				--[visibility_component.CONTAINMENT] = {
				--	square_side = 250,
				--	color = rgba(0, 255, 255, 120),
				--	ignore_discontinuities_shorter_than = -1,
				--	filter = filter_obstacle_visibility
				--}
			}
		},
		      
		particle_emitter = {
			available_particle_effects = npc_effects
		},
		
		pathfinding = {
			enable_backtracking = true,
			target_offset = 100,
			rotate_navpoints = 10,
			distance_navpoint_hit = 2,
			favor_velocity_parallellness = true
		},
		
		movement = {
			input_acceleration = vec2(30000, 30000),
			max_speed = 3300,
			max_speed_animation = 2300,
			
			receivers = {
				{ target = "body", stop_at_zero_movement = false }, 
				{ target = "legs", stop_at_zero_movement = true  }
			}
		},
		
		steering = {
			max_resultant_force = -1 -- -1 = no force clamping
		},
		 
		children = {
			"legs"
		},
		
		scriptable = {
			available_scripts = npc_damage_handler
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
			available_animations = player_animations.sets.legs
		}
	}
}

npc_wield_offsets = {
	HEAD = {
		shot = {
			{ pos = vec2(0, 0)  },
			{ pos = vec2(-1, 0)  },
			{ pos = vec2(-2, 0)  },
			{ pos = vec2(-4, 0)  },
			{ pos = vec2(-6, 0)  }
		},
		
		walk = {
			{ pos = vec2(0, 0) },
			{ pos = vec2(-1, 0) },
			{ pos = vec2(-2, 0) },
			{ pos = vec2(-3, 0) },
			{ pos = vec2(-4, 0) }
		},
		
		walk_cw = {
			{ pos = vec2(0, 0)  },
			{ pos = vec2(1, 0)  },
			{ pos = vec2(2, 0)  },
			{ pos = vec2(3, 0)  },
			{ pos = vec2(4, 0)  }
		},
		
		swing = {
			{ pos = vec2(0, 0)   },
			{ pos = vec2(0, 1)   },
			{ pos = vec2(1, 2)  },
			{ pos = vec2(0, 3)   },
			{ pos = vec2(0, 4)   }
		},
		
		swing_cw = {
			{ pos = vec2(0, 0) },
			{ pos = vec2(0, -1) },
			{ pos = vec2(-1, -2) },
			{ pos = vec2(0,  -3) },
			{ pos = vec2(0,  -4) }
		}
	
	},
	
	FIREAXE = {
		walk = {
			{ rotation = 180, pos = vec2(7, 24) },
			{ rotation = 180, pos = vec2(6, 24) },
			{ rotation = 180, pos = vec2(7, 23) },
			{ rotation = 180, pos = vec2(6, 24) },
			{ rotation = 180, pos = vec2(7, 24) }
		},
		
		walk_cw = {
			{ rotation = 0, pos = vec2(7, -24), flip = true },
			{ rotation = 0, pos = vec2(6, -24), flip = true },
			{ rotation = 0, pos = vec2(7, -23), flip = true },
			{ rotation = 0, pos = vec2(6, -24), flip = true },
			{ rotation = 0, pos = vec2(7, -24), flip = true }
		},
		
		swing = {
			{ rotation = 150, pos = vec2(-5, 5)   + vec2.rotated(vec2(7, 24), vec2(17, -10), -30 ) },
			{ rotation = 120, pos = vec2(-8, 7)   + vec2.rotated(vec2(7, 24), vec2(17, -10), -60 ) },
			{ rotation = 90,  pos = vec2(-10, 10) + vec2.rotated(vec2(7, 24), vec2(17, -10), -90 )  },
			{ rotation = 60,  pos = vec2(-12, 12) + vec2.rotated(vec2(7, 24), vec2(17, -10), -120 )  },
			{ rotation = 30,  pos = vec2(-15, 15) + vec2.rotated(vec2(7, 24), vec2(17, -10), -150 )  }
		},
		
		swing_cw = {
			{ rotation = 30,  pos = vec2(-15, -15) + vec2.rotated(vec2(7, 24), vec2(17, -10), -150 ) , flip = true },
			{ rotation = 60,  pos = vec2(-12, -12) + vec2.rotated(vec2(7, 24), vec2(17, -10), -120 ) , flip = true },
			{ rotation = 90,  pos = vec2(-10, -10) + vec2.rotated(vec2(7, 24), vec2(17, -10), -90 ) , flip = true },
			{ rotation = 120, pos = vec2(-8, -7)   + vec2.rotated(vec2(7, 24), vec2(17, -10), -60 ), flip = true },
			{ rotation = 150, pos = vec2(-5, -5)   + vec2.rotated(vec2(7, 24), vec2(17, -10), -30 ), flip = true }
		}
	},
	
	SHOTGUN = {
		walk = {
			{ rotation = 0, pos = vec2(35, 10) },
			{ rotation = 0, pos = vec2(36, 10) },
			{ rotation = 0, pos = vec2(37, 10) },
			{ rotation = 0, pos = vec2(39, 10) },
			{ rotation = 0, pos = vec2(40, 10) }
		},
		
		shot = {
			{ rotation = 0, pos = vec2(34, 10)  },
			{ rotation = 0, pos = vec2(33, 10)  },
			{ rotation = 0, pos = vec2(32, 10)  },
			{ rotation = 0, pos = vec2(30, 10)  },
			{ rotation = 0, pos = vec2(28, 10)  }
		}
	},
	
	ASSAULT_RIFLE = {
		walk = {
			{ rotation = 0, pos = vec2(35, 10) },
			{ rotation = 0, pos = vec2(36, 10) },
			{ rotation = 0, pos = vec2(37, 10) },
			{ rotation = 0, pos = vec2(39, 10) },
			{ rotation = 0, pos = vec2(40, 10) }
		},
		
		shot = {
			{ rotation = 0, pos = vec2(34, 10)  },
			{ rotation = 0, pos = vec2(33, 10)  },
			{ rotation = 0, pos = vec2(32, 10)  },
			{ rotation = 0, pos = vec2(30, 10)  },
			{ rotation = 0, pos = vec2(28, 10)  }
		}
	}
}

--npc_wield_offsets.ASSAULT_RIFLE = archetyped(npc_wield_offsets.SHOTGUN, {})

dofile "scripts\\sample_scenes\\npc.lua"
dofile "scripts\\sample_scenes\\player.lua"

loop_only_info = create_scriptable_info {
	scripted_events = {	
		[scriptable_component.INTENT_MESSAGE] = 
			function(message)
				if message.intent == custom_intents.QUIT then
					input_system.quit_flag = 1
				elseif message.intent == custom_intents.RESTART then
					print "INTENT.."
					if not player.body:exists() then
					print "RELOADING.."
						set_world_reloading_script(entities)
					end
				elseif message.intent == custom_intents.DROP_WEAPON then
					if message.state_flag then
						if player.body:exists() then get_scripted(player.body:get()):pick_up_weapon() end
					end
				elseif message.intent == custom_intents.INSTANT_SLOWDOWN then
					physics_system.timestep_multiplier = 0.00001
				elseif message.intent == custom_intents.SPEED_INCREASE then
					physics_system.timestep_multiplier = physics_system.timestep_multiplier + 0.05
				elseif message.intent == custom_intents.SPEED_DECREASE then
					physics_system.timestep_multiplier = physics_system.timestep_multiplier - 0.05
					
					if physics_system.timestep_multiplier < 0.01 then
						physics_system.timestep_multiplier = 0.01
					end
				end
				return true
			end,
			
		[scriptable_component.LOOP] = function(subject)
			my_atlas:_bind()
			
			for k, v in ipairs(my_npcs) do 
				if v.body:exists() then 
					get_scripted(v.body:get()):loop()
				end 
			end
			
			if player.body:exists() then
				local player_info = get_scripted(player.body:get())
			
				if player.body:get().gun.current_state == gun_component.SWINGING or player.body:get().gun.current_state == gun_component.SHOOTING_INTERVAL then
					player_info.head_entity.render.model = head_shot_sprite
				elseif player_info.current_weapon.animation_index == "BARE_HANDS" then
					player_info.head_entity.render.model = head_walk_sprite
				else
					player_info.head_entity.render.model = head_gun_sprite
				end
				
				player_info:loop()
			end
			
			--local ppos = player.body:get().transform.current.pos
			----render_system:push_non_cleared_line(debug_line(vec2(0, 0), vec2(1000, 1000),  rgba(0, 255, 0, 255)))
			--render_system:push_line(debug_line(ppos, ppos+vec2(7, 24), rgba(0, 0, 255, 255)))
			--render_system:push_line(debug_line(ppos, ppos+vec2(17, -10), rgba(0, 255, 0, 255)))
			--
			--for i=1, 90 do
			--render_system:push_line(debug_line(ppos, ppos+vec2.rotated(vec2(7, 24), vec2(17, -10), -i), rgba(255, 0, 0, 255)))
			--end
		end
	}
}

myloopscript = create_entity {
	input = {
			custom_intents.SPEED_INCREASE,
			custom_intents.SPEED_DECREASE,
			custom_intents.INSTANT_SLOWDOWN,
			custom_intents.QUIT,
			custom_intents.RESTART,
			custom_intents.DROP_WEAPON
	},
		
	scriptable = {
		available_scripts = loop_only_info,
		script_data = {}
	}
}
