-- gets map filename and scene object to save global entities like players/cameras

return function(map_filename, scene_object)
	-- setup shortcut
	local world = scene_object.world_object
	
	scene_object.simulation_world = simulation_world_class:create()
	
	-- load map data
	scene_object.resource_storage = {}
	create_particle_effects(scene_object)


	world.physics_system.enable_interpolation = 1
	world.physics_system:configure_stepping(config_table.tickrate, 5)
	scene_object.simulation_world.physics_system:configure_stepping(config_table.tickrate, 5)
	
	-- initialize input
	world.input_system:clear_contexts()
	world.input_system:add_context(main_input_context)
	world.input_system:add_context(gui_input_context)
	
	-- initialize camera
	scene_object.world_camera = create_world_camera_entity(world, scene_object.sprite_library["blank"])
	scene_object.world_camera.script.owner_scene = scene_object
	scene_object.world_camera.transform.pos = vec2(100, 100)
	
	--if config_table.multiple_clients_view == 1 then
	--	scene_object.world_camera.script.min_zoom = -400
	--	scene_object.world_camera.script:set_zoom_level(-400)
	--end
	
	-- initialize player
	scene_object.teleport_position = vec2(0, 0)--objects_by_type["teleport_position"][1].pos
	
	scene_object.crosshair_sprite = create_sprite {
		image = scene_object.sprite_library["crosshair"],
		color = rgba(0, 255, 255, 255)
	}
	
	scene_object.bullet_sprite = create_sprite {
		image = scene_object.sprite_library["bullet"],
		size_multiplier = vec2(3, 0.4),
		color = rgba(0, 255, 255, 255)
	}	

	scene_object.pink_bullet_sprite = create_sprite {
		image = scene_object.sprite_library["bullet"],
		size_multiplier = vec2(3, 0.4),
		color = rgba(255, 0, 255, 255)
	}
	
	
	scene_object.legs_sets = create_all_legs_sets(scene_object.sprite_library)
	scene_object.torso_sets = create_all_torso_sets(scene_object.sprite_library)
	
	scene_object.simulation_player = create_simulation_player(scene_object.simulation_world)
	
	scene_object.main_input = world:create_entity {
		input = {
			custom_intents.SWITCH_CLIENT_1,
			custom_intents.SWITCH_CLIENT_2,
			custom_intents.SWITCH_CLIENT_3,
			custom_intents.SWITCH_CLIENT_4,
			
			custom_intents.QUIT
		},
		
		script = {
			intent_message = function(self, message)
				if message.intent == custom_intents.QUIT then
					SHOULD_QUIT_FLAG = true
				elseif message.intent == custom_intents.SWITCH_CLIENT_1 then
					set_active_client(1)
				elseif message.intent == custom_intents.SWITCH_CLIENT_2 then
					set_active_client(2)
				elseif message.intent == custom_intents.SWITCH_CLIENT_3 then
					set_active_client(3)
				elseif message.intent == custom_intents.SWITCH_CLIENT_4 then
					set_active_client(4)
				end		
			end
		}
	}
	
	local all_sound_files = get_all_files_in_directory("hypersomnia\\data\\sfx")
	local sound_by_filename = {}
	local sound_library = {}
	
	for k, v in pairs(all_sound_files) do
		local sound_object = create_sound("hypersomnia\\data\\sfx\\" .. v)
		
		sound_by_filename[k] = sound_object
		
		-- tokenize filename to only get the filename and the extension
		local tokenized = tokenize_string(v, "\\/")
		
		-- the last token is just filename + extension
		save_resource_in_item_library(tokenized[#tokenized], sound_object, sound_library)
	end
	
	scene_object.sound_library = sound_library
	scene_object.sound_by_filename = sound_by_filename
	
	-- bind the atlas once
	-- GL.glActiveTexture(GL.GL_TEXTURE0)
	-- scene_object.all_atlas:bind()
	-- now have to bind every time because rendering several clients

	local visibility_system = world.visibility_system
	local pathfinding_system = world.pathfinding_system
	local render_system = world.render_system

	--visibility_system.draw_cast_rays = 0
	--visibility_system.draw_triangle_edges = 0
	--visibility_system.draw_discontinuities = 0
	--visibility_system.draw_visible_walls = 0
	
	visibility_system.epsilon_ray_distance_variation = 0.001
	visibility_system.epsilon_threshold_obstacle_hit = 2
	visibility_system.epsilon_distance_vertex_hit = 1
	
	--pathfinding_system.draw_memorised_walls = 0
	--pathfinding_system.draw_undiscovered = 0
	--pathfinding_system.epsilon_max_segment_difference = 4
	--pathfinding_system.epsilon_distance_visible_point = 2
	--pathfinding_system.epsilon_distance_the_same_vertex = 10
	--
	--render_system.debug_drawing = 1
	--
	--render_system.draw_steering_forces = 1
	--render_system.draw_substeering_forces = 1
	--render_system.draw_velocities = 1
	--
	--render_system.draw_avoidance_info = 1
	--render_system.draw_wandering_info = 1
	--
	--render_system.visibility_expansion = 0.0
	--render_system.max_visibility_expansion_distance = 0
	--render_system.draw_visibility = 0

	scene_object.load_objects = function()

	create_animations(scene_object, scene_object.sprite_library)

	local objects_by_type, type_table_by_object = tiled_map_loader.get_all_objects_by_type(map_filename)

	-- helper function for getting all objects of given type
	local function get_all_objects(entries)
		local sum_of_all = {}
		for i = 1, #entries do
			sum_of_all = table.concatenate( { sum_of_all, objects_by_type[entries[i]] } )
		end
		
		return sum_of_all
	end
	
	
	local function basic_table(object)
		return tiled_map_loader.basic_entity_table(object, type_table_by_object[object], scene_object.resource_storage, scene_object.world_camera, scene_object.texture_by_filename)
	end
	
	local background_objects = get_all_objects { "ground_snow" }
	for i = 1, #background_objects do
		local object = background_objects[i]
		world:create_entity (basic_table(object))
	end
	

	
	scene_object:load_tile_functionality(map_filename)

	scene_object.tile_layers = {}

	tiled_map_loader.for_every_object(map_filename, nil, function(tile_layer_table)
		local new_model = scene_object:generate_tile_layer(tile_layer_table)
		
		world:create_entity {
			render = {
				model = new_model,
				layer = render_layers["GROUND"]
			},

			transform = {}
		}

		scene_object.tile_layers[#scene_object.tile_layers+1] = new_model
	end)
	
	local fire_objects = get_all_objects { "fire" }
	
	for i = 1, #fire_objects do
		local burst = particle_burst_message()
		local light_filter = create_query_filter({"STATIC_OBJECT"})

		local new_fire_entity = world:create_entity {
			transform = {
				pos = fire_objects[i].pos + vec2(10, 10),
				rotation = fire_objects[i].rotation
			},

			visibility = {
				interval_ms = 32,
				visibility_layers = {
					[visibility_layers.BASIC_LIGHTING] = {
						square_side = 1500,
						color = rgba(0, 255, 255, 10),
						ignore_discontinuities_shorter_than = -1,
						filter = light_filter
					}
				}
			}
		}

		burst.local_transform = true
		burst.subject = new_fire_entity
		burst.type = particle_burst_message.CUSTOM
		burst:set_effect(scene_object.particles.fire_effect)
		
		local new_light_entity = components.create_components {
			cpp_entity = new_fire_entity,

			light = {
				color = rgba(255, 255, 0, 255),

				attenuation = {
					0.7,
					0.0,
					0.00027,
					750
				},

				wall_attenuation = {
					1,
					0.0,
					0.00037,
					750
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
						change_speed = 0.00027
					},
		
					{
						value = 0,
						min_value = -0.00005,
						max_value = 0.00030,
						change_speed = 0.00067
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

		scene_object.world_object.world:post_message(burst)
		scene_object.owner_client_screen.entity_system_instance:add_entity(new_light_entity)
	end

		-- initialize environmental physical objects
	local environmental_objects = get_all_objects { "cathedral_wall" }
	
	for i = 1, #environmental_objects do
		local object = environmental_objects[i]
		
		local new_entity_table = basic_table(object)

		local new_entity = world:create_entity (new_entity_table)

		scene_object.simulation_world:create_entity {
			transform = new_entity_table.transform,
			physics = new_entity_table.physics
		}

		local new_wall_entity = components.create_components {
			cpp_entity = new_entity,
			
			particle_response = {
				response = scene_object.particles.metal_response
			}
		}

		scene_object.owner_client_screen.entity_system_instance:add_entity(new_wall_entity)

	end

	end
end