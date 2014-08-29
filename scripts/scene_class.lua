scene_class = inherits_from ()

function scene_class:constructor()

end

function scene_class:load_map(map_filename, map_loader_filename, all_fonts)
	self.world_object = world_class:create()
	
	local gameplay_textures_directory = tiled_map_loader.try_to_load_map(map_filename).properties["gameplay_textures"] 
	
	if gameplay_textures_directory == nil then
		print ("gameplay_textures property unspecified in " .. map_filename)
	end
	
	-- concatenate table with gameplay textures and table with map textures
	local all_needed_textures = table.concatenate({ 
		get_all_files_in_directory(remove_filename_from_path(map_filename) .. gameplay_textures_directory, true), 
		tiled_map_loader.get_all_textures(map_filename)
	})

	-- create texture atlas
	local all_atlas, sprite_library, sprite_object_library, texture_by_filename, font_files, font_by_name = create_atlas_from_filenames(all_needed_textures, all_fonts)
	
	self.all_atlas = all_atlas
	self.sprite_library = sprite_library
	self.sprite_object_library = sprite_object_library
	self.texture_by_filename = texture_by_filename
	
	self.font_files = font_files
	self.font_by_name = font_by_name
	
	-- the loader creates all the entities in the current world
	dofile(map_loader_filename)(map_filename, self)
end