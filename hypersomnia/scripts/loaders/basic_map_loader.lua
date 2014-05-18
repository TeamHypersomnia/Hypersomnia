-- gets map filename and scene object to save global entities like players/cameras

return function(map_filename, scene_object)
	input_system:clear_contexts()
	input_system:add_context(main_input_context)
	
	scene_object.world_camera = create_world_camera_entity()
	get_self(scene_object.world_camera).owner_scene = scene_object
	
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
	
	local environmental_objects = get_all_objects { "wall_wood", "crate" }
	
	for i = 1, #environmental_objects do
		local object = environmental_objects[i]
		create_entity (tiled_map_loader.basic_entity_table(object, type_table_by_object[object], scene_object.resource_storage, scene_object.world_camera, scene_object.texture_by_filename))
	end
	
	
	-- bind the atlas once
	GL.glActiveTexture(GL.GL_TEXTURE0)
	scene_object.all_atlas:bind()
end