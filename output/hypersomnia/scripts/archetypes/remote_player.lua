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
			
			gun = {}, 
	
			particle_emitter = {
				available_particle_effects = owner_scene.particles.npc_effects
			},
			
			movement = {
				input_acceleration = vec2(10000, 10000),
				max_accel_len = 10000,
				max_speed_animation = 1000,
				air_resistance = 0.5,
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
				layer = render_layers.CROSSHAIRS,
				model = crosshair_sprite
			},
			
			crosshair = {
				sensitivity = config_table.sensitivity,
				should_blink = false
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


world_archetype_callbacks.REMOTE_PLAYER = {
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
			
			wield = {
				wield_offsets = npc_wield_offsets
			},
			
			label = {
				position = components.label.positioning.OVER_HEALTH_BAR,
				default_font = self.owner_scene.font_by_name["kubasta"],
				
				text = {}
			}
		}
		
		new_entity.wield.on_item_wielded = function(this, picked, old_item, wielding_key)
			if wielding_key == components.wield.keys.PRIMARY_WEAPON then
				new_remote_player.body.animate.available_animations = self.owner_scene.torso_sets["white"][picked.item.outfit_type].set

				new_remote_player.body.movement.animation_message = animation_events.MOVE
				
				if picked.weapon ~= nil then
					if picked.weapon.is_melee then
						if picked.weapon.current_swing_direction then
							new_remote_player.body.movement.animation_message = animation_events.MOVE_CW
						else
							new_remote_player.body.movement.animation_message = animation_events.MOVE_CCW
						end
					end
					
					
					picked.weapon.transmit_bullets = false
					picked.weapon.constrain_requested_bullets = false
					picked.weapon.bullet_entity.physics.body_info.filter = filters.REMOTE_BULLET
				end
				
				local stop_msg = animate_message()
				stop_msg.subject = new_remote_player.body
				stop_msg.message_type = animate_message.STOP
				stop_msg.animation_priority = 100
				
				local msg = animate_message()
				msg.subject = new_remote_player.body
				msg.message_type = animate_message.START
				msg.change_speed = true
				msg.animation_type = new_remote_player.body.movement.animation_message
				msg.speed_factor = 0
				msg.animation_priority = 0
				
				new_remote_player.body.owner_world:post_message(stop_msg)
				new_remote_player.body.owner_world:post_message(msg)
				
			end
		end
		
		new_entity.wield.on_item_unwielded = function(this, old_item, wielding_key)
			if wielding_key == components.wield.keys.PRIMARY_WEAPON then
				new_remote_player.body.movement.animation_message = animation_events.MOVE
				new_remote_player.body.animate.available_animations = self.owner_scene.torso_sets["white"]["barehands"].set
				
				local stop_msg = animate_message()
				stop_msg.subject = new_remote_player.body
				stop_msg.message_type = animate_message.STOP
				stop_msg.animation_priority = 100
			end
		end
		
		return new_entity
	end
}


