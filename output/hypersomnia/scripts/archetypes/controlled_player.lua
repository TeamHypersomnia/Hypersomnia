local player_physics_component = {
	body_type = Box2D.b2_dynamicBody,
	
	body_info = {
		filter = filters.CHARACTER,
		shape_type = physics_info.RECT,
		rect_size = vec2(37, 37),
		
		angular_damping = 5,
		--linear_damping = 18,
		--max_speed = 3300,
		
		fixed_rotation = true,
		density = 0.1
	}
}
			
local player_movement_component = {
	input_acceleration = vec2(8000, 8000),
	max_speed_animation = 800,
	air_resistance = 0.05,
	braking_damping = 18,
	receivers = {
		{ target = "body", stop_at_zero_movement = false }, 
		{ target = "legs", stop_at_zero_movement = true  }
	}
}

function create_simulation_player(owner_world)
	return owner_world:create_entity {
		transform = {},
		physics = player_physics_component,
		movement = player_movement_component
	}
end

function create_controlled_player(scene_object, position, target_camera, crosshair_sprite)
	local player = scene_object.world_object:create_entity_group  {
		-- body also acts as torso
		body = {
			render = {
				layer = render_layers.PLAYERS,
				model = blank_green
			},
		
			transform = {
				pos = position
			},
			
			animate = {
			
			},
					
			physics = player_physics_component,
			
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
	
			particle_emitter = {
				available_particle_effects = scene_object.particles.npc_effects
			},
			
			movement = player_movement_component,
		 
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
			},
			
			input = {
				intent_message.AIM
			}
		},
		
		legs = {
			transform = { 
				pos = position,
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
	
	if target_camera ~= nil then
		target_camera.chase:set_target(player.body)
		target_camera.camera.player:set(player.body)
		target_camera.camera.crosshair:set(player.crosshair)
	end
	
	player.body.animate.available_animations = scene_object.torso_sets["white"]["rifle"].set
	player.legs.animate.available_animations = scene_object.legs_sets["white"].set
	
	return player
end


