-- gets map filename and scene object to save global entities like players/cameras

return function(map_filename, scene_object)
	-- setup shortcut
	local world = scene_object.world_object
	
	-- load map data
	scene_object.resource_storage = {}
	local objects_by_type, type_table_by_object = tiled_map_loader.get_all_objects_by_type(map_filename)

	-- helper function for getting all objects of given type
	local function get_all_objects(entries)
		local sum_of_all = {}
		for i = 1, #entries do
			sum_of_all = table.concatenate( { sum_of_all, objects_by_type[entries[i]] } )
		end
		
		return sum_of_all
	end
	
	-- initialize environmental physical objects
	local environmental_objects = get_all_objects { "wall_wood", "crate" }
	
	for i = 1, #environmental_objects do
		local object = environmental_objects[i]
		world:create_entity (tiled_map_loader.basic_entity_table(object, type_table_by_object[object], scene_object.resource_storage, scene_object.world_camera, scene_object.texture_by_filename))
	end
	
	
	
	
	-- initialize input
	world.input_system:clear_contexts()
	world.input_system:add_context(main_input_context)
	
	-- initialize camera
	scene_object.world_camera = create_world_camera_entity(world)
	get_self(scene_object.world_camera).owner_scene = scene_object
	
	-- initialize player
	local player_start = objects_by_type["teleport_position"][1]
	
	scene_object.crosshair_sprite = create_sprite {
		image = scene_object.sprite_library["crosshair"],
		color = rgba(255, 255, 255, 255)
	}
	
	scene_object.player = create_basic_player(world, player_start.pos, scene_object.world_camera, scene_object.crosshair_sprite)
	scene_object.legs_sets = create_all_legs_sets(scene_object.sprite_library)
	scene_object.torso_sets = create_all_torso_sets(scene_object.sprite_library)

	scene_object.player.body:get().animate.available_animations = scene_object.torso_sets["white"]["barehands"].set
	scene_object.player.legs:get().animate.available_animations = scene_object.legs_sets["white"].set

	scene_object.main_input_entity = world:create_entity_table(world:ptr_create_entity {
		input = {
			custom_intents.SWITCH_CLIENT_1,
			custom_intents.SWITCH_CLIENT_2,
			custom_intents.SWITCH_CLIENT_3,
			custom_intents.SWITCH_CLIENT_4,
			
			custom_intents.QUIT
		}
	})
	
	scene_object.main_input_entity.intent_message = function(self, message)
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
	
	
	-- bind the atlas once
	-- GL.glActiveTexture(GL.GL_TEXTURE0)
	-- scene_object.all_atlas:bind()
	-- now have to bind every time because rendering several clients
end