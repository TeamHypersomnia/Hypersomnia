scene_class = inherits_from ()

function scene_class:constructor()

end

function scene_class:load_map(map_filename, map_loader_filename)
	self.world_object = world_class:create()
	
	local gameplay_textures_directory = tiled_map_loader.try_to_load_map(map_filename).properties["gameplay_textures"] 
	
	if gameplay_textures_directory == nil then
		print ("gameplay_textures property unspecified in " .. map_filename)
	end
	
	-- concatenate table with gameplay textures and table with map textures
	local all_needed_textures = table.concatenate({ 
		get_all_files_in_directory(gameplay_textures_directory, true), 
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

function scene_class:set_current()
	self.world_object:set_current()
end