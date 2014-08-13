function create_remote_player(owner_scene, crosshair_sprite)
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
					max_speed = 3300,
					
					fixed_rotation = true,
					density = 0.1
				},
			},
			
			lookat = {
				target = "crosshair",
				look_mode = lookat_component.POSITION,
				
				easing_mode = lookat_component.EXPONENTIAL,
				smoothing_average_factor = 0.5,	
				averages_per_sec = 80	
			},
			
			gun = {}, 
	
			particle_emitter = {
				available_particle_effects = owner_scene.particles.npc_effects
			},
			
			movement = {
				input_acceleration = vec2(5000, 5000),
				max_speed_animation = 1000,
				air_resistance = 0.1,
				braking_damping = 18,
				receivers = {
					{ target = "body", stop_at_zero_movement = false }, 
					{ target = "legs", stop_at_zero_movement = true  }
				}
			},
		 
			children = {
				"legs",
				"crosshair"
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
				sensitivity = config_table.sensitivity
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
	
	player.body.animate.available_animations = owner_scene.torso_sets["white"]["barehands"].set
	player.legs.animate.available_animations = owner_scene.legs_sets["white"].set

	return player
end


world_archetype_callbacks["REMOTE_PLAYER"] = {
	creation = function(self)
		local new_remote_player = create_remote_player(self.owner_scene, self.owner_scene.crosshair_sprite)
	
		local new_entity = components.create_components {
			cpp_entity = new_remote_player.body,
			interpolation = {},
			
			orientation = {
				receiver = true,
				crosshair_entity = new_remote_player.crosshair
			},
			
			health = {},
			
			wield = {}
		}
		
		new_entity.wield.on_item_selected = function(this, picked)
			new_remote_player.body.animate.available_animations = self.owner_scene.torso_sets["white"][picked.item.wield_type].set
			
			if picked.weapon ~= nil then
				picked.weapon.transmit_bullets = false
				picked.weapon.constrain_requested_bullets = false
				picked.weapon.bullet_entity.physics.body_info.filter = filters.REMOTE_BULLET
			end
		end
		
		new_entity.wield.on_drop = function(this)
			new_remote_player.body.animate.available_animations = self.owner_scene.torso_sets["white"]["barehands"].set
		end
		
		return new_entity
	end
}


