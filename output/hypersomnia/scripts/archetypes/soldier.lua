function create_soldier(owner_scene, crosshair_sprite)
	local light_filter = create_query_filter({"STATIC_OBJECT"})

	local player = owner_scene.world_object:create_entity_group  {
		-- body also acts as torso
		body = {
			render = {
				layer = render_layers.PLAYERS,
				model = blank_green
			},
		
			transform = {
			},
			
			animate = {
			
			},
					
			physics = {
				body_type = Box2D.b2_dynamicBody,
				
				body_info = {
					filter = filters.REMOTE_CHARACTER,
					shape_type = physics_info.RECT,
					rect_size = vec2(37, 37),
					
					angular_damping = 5,
					--linear_damping = 18,
					--max_speed = 3300,
					
					fixed_rotation = true,
					density = 0.1,
					angled_damping = true
				},
			},
			
			lookat = {
				target = "crosshair",
				look_mode = lookat_component.POSITION,
				
				easing_mode = lookat_component.EXPONENTIAL,
				smoothing_average_factor = 0.5,	
				averages_per_sec = 80	
			},
			
			particle_emitter = {
				available_particle_effects = owner_scene.particles.npc_effects
			},
			
			movement = {
				input_acceleration = vec2(5000, 5000),
				max_accel_len = 5000,
				max_speed_animation = 200,
				air_resistance = 0.6,
				braking_damping = 18,
				receivers = {
					{ target = "body", stop_at_zero_movement = false }, 
					{ target = "legs", stop_at_zero_movement = true  }
				}
			},
		 
			children = {
				"legs",
				"crosshair"
			},

			visibility = {
				interval_ms = 32,
				visibility_layers = {
					[visibility_layers.BASIC_LIGHTING] = {
						square_side = 200,
						color = rgba(0, 255, 255, 10),
						ignore_discontinuities_shorter_than = -1,
						filter = light_filter
					}
				}
			}
		},
		
		crosshair = { 
			transform = {
				pos = vec2(0, 0),
				rotation = 0
			},
			
			render = {
				layer = render_layers.CROSSHAIRS,
				model = nil
			},

			chase = {
				target = "body",
				relative = true
			}
		},
		
		legs = {
			transform = { 
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
			
			}
		}
	}
	
	player.body.animate.available_animations = owner_scene.torso_sets["basic"]["barehands"].set
	player.legs.animate.available_animations = owner_scene.legs_sets["basic"].set

	return player
end


world_archetype_callbacks.SOLDIER = {
	creation = function(self)
		local soldier_group = create_soldier(self.owner_scene, self.owner_scene.crosshair_sprite)
	
		local new_entity = components.create_components {
			cpp_entity = soldier_group.body,
			interpolation = {},
			
			orientation = {
				receiver = true,
				crosshair_entity = soldier_group.crosshair
			},
			
			health = {},
			
			wield = {
				wield_offsets = npc_wield_offsets
			},
			
			label = {
				position = components.label.positioning.OVER_HEALTH_BAR,
				default_font = self.owner_scene.font_by_name["kubasta"],
				
				text = {
					{
						str = "Goliathus",
						color = rgba(255, 255, 255, 255)
					}
				}
			},

			light = {
				color = rgba(0, 255, 0, 255),

				attenuation = {
					1,
					0.01,
					0.0027,
					80
				},

				wall_attenuation = {
					1,
					0.01,
					0.0037,
					80
				},

				attenuation_variations = 
				{
					{
						value = 0,
						min_value = -0.3,
						max_value = 0.0,
						change_speed = 0.0/5
					},
			
					{
						value = 0,
						min_value = -0.00001,
						max_value = 0.00002,
						change_speed = 0.0000
					},
		
					{
						value = 0,
						min_value = -0.00005,
						max_value = 0.00030,
						change_speed = 0.0000
					},
	
				-- light position variation
					{
						value = 0,
						min_value = -10,
						max_value = 10,
						change_speed = 50
					},
		
					{
						value = 0,
						min_value = -5,
						max_value = 5,
						change_speed = 50
					}
				}
			}
		}
		
		new_entity.wield.on_item_wielded = function(this, picked, old_item, wielding_key)
			return character_wielding_procedure(self.owner_scene, soldier_group, false, this, picked, old_item, wielding_key)
		end
		
		new_entity.wield.on_item_unwielded = function(this, unwielded, wielding_key)
			return character_unwielding_procedure(self.owner_scene, soldier_group, false, this, unwielded, wielding_key)
		end
		
		return new_entity
	end
}


