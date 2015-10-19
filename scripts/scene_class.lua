scene_class = inherits_from ()

function scene_class:constructor()

end

function scene_class:load_tile_functionality(map_filename)
	self.tile_layer_library = {}

	_, self.tile_id_to_texture_filename = tiled_map_loader.get_tileset_textures(map_filename) 
	self.map_tileset = tileset()

	for tile_id=1, #self.tile_id_to_texture_filename do
		local tile_filename = self.tile_id_to_texture_filename[tile_id]

		self.map_tileset.tile_types:add(tile_type(self.texture_by_filename[tile_filename].tex))
	end
end

function scene_class:generate_tile_layer(tile_layer_table)
	--tile_layer_table.width = 2
	--tile_layer_table.height = 2
	local new_tile_layer = tile_layer(rect_wh_i(tile_layer_table.width, tile_layer_table.height))
	new_tile_layer.layer_tileset = self.map_tileset

	new_tile_layer.tiles:reserve(#tile_layer_table.data)
	
	for i=1, #tile_layer_table.data do
		new_tile_layer.tiles:add(tile_object(tile_layer_table.data[i]))
	end 

	table.insert(self.tile_layer_library, new_tile_layer)

	return new_tile_layer
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
	local all_atlas, 
	sprite_library, 
	sprite_object_library, 
	texture_by_filename, 
	font_files, 
	font_by_name = create_atlas_from_filenames(all_needed_textures, all_fonts)

	self.all_atlas = all_atlas
	self.sprite_library = sprite_library
	self.sprite_object_library = sprite_object_library
	self.texture_by_filename = texture_by_filename
	
	self.font_files = font_files
	self.font_by_name = font_by_name
	
	-- the loader creates all the entities in the current world
	dofile(map_loader_filename)(map_filename, self)
end