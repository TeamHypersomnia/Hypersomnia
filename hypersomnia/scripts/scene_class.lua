scene_class = inherits_from ()

function scene_class:constructor()

end

function scene_class:load_map_and_set_current(map_filename, map_loader_filename)
	self.world_object = world_class:create()
	self.world_object:set_current()
	
	-- concatenate table with gameplay textures and table with map textures
	local all_needed_textures = table.concatenate({ 
		get_all_files_in_directory(tiled_map_loader.try_to_load_map(map_filename).properties["gameplay_textures"], true), 
		tiled_map_loader.get_all_textures(map_filename)
	})

	-- create texture atlas
	local all_atlas, sprite_library, texture_by_filename = create_atlas_from_filenames(all_needed_textures)
	
	self.all_atlas = all_atlas
	self.sprite_library = sprite_library
	self.texture_by_filename = texture_by_filename
	
	-- the loader creates all the entities in the current world
	require(map_loader_filename)(map_filename, self)
end