tiled_map_loader = {
	error_callback = function(msg)
		print (msg)
		debugger_break()
	end,
	
	world_camera_entity = 0,
	
	texture_property_name = "texture",
	
	type_library = "sensibilia/maps/object_types",
	world_information_library = "sensibilia/maps/world_properties",
	
	map_scale = 1.2,
	allow_unknown_types = true,
	
	for_every_object = function(filename, callback)
		local this = tiled_map_loader
		local err = this.error_callback
		
		-- get them by copy
		local map_table = archetyped(require(filename), {})
		local type_table = archetyped(require(this.type_library), {})
		
		if type_table == nil then 
			err ("error loading type table " .. this.type_library)
		end
		
		if map_table == nil then 
			err ("error loading map filename " .. filename)
		end
		
		if type_table.default_type == nil then 
			type_table.default_type = {}
		end
		
		local map_center_translation = vec2(map_table.width*map_table.tilewidth, map_table.height*map_table.tileheight) / 2
					
		for a, layer in ipairs(map_table.layers) do
			if layer.type == "objectgroup" then
				for b, object in ipairs(layer.objects) do
					if object.type == "" then
						object.type = "default_type"
					end
				
					local output_type_table = {}
					
					-- perform type operations only for non-information entities
					if require(this.world_information_library)[object.type] == nil then
						if this.allow_unknown_types and type_table[object.type] == nil then
							type_table[object.type] = {}
						end
					
						local this_type_table = type_table[object.type]
						
						if this_type_table == nil then
							err ("couldn't find type " .. object.type .. " for object \"" .. object.name .. "\" in layer \"" .. layer.name .. "\"")
							--print ("couldn't find type " .. object.type .. " for object \"" .. object.name .. "\" in layer \"" .. layer.name .. "\"")
						end
						
						-- validation
						if this_type_table.entity_archetype == nil then
							--err("unspecified entity archetype for type " .. object.type)
							--print("unspecified entity archetype for type " .. object.type)
							this_type_table.entity_archetype = {}
						end
					
						-- property priority (lowest to biggest):
						-- layer properties set in Tiled
						-- type properties from type library
						-- object-specific properties set in Tiled
						
						-- could be written in one line but separated for clarity
						output_type_table = archetyped(layer.properties, this_type_table)
						output_type_table = archetyped(output_type_table, object.properties)
						
						-- rest of the validations (after the properties have been overridden)
					end
					
					-- handle object scale now, to simplify further calculations
					
					object.x = (object.x - map_center_translation.x) * this.map_scale
					object.y = (object.y - map_center_translation.y) * this.map_scale
					object.width = object.width * this.map_scale
					object.height = object.height * this.map_scale
					
					if object.polygon ~= nil then
						for k, v in ipairs(object.polygon) do
							object.polygon[k] = vec2(v.x * this.map_scale, v.y * this.map_scale)
						end
					end
					
					-- convenience field
					object.pos = vec2(object.x, object.y)
			
					-- callback
					callback(object, output_type_table)
				end
			end
		end
		
		return needed_textures
	
	end,
	
	get_all_objects_by_type = function(filename)
		local this = tiled_map_loader
		local all_objects_by_type = {}
		local type_tables_by_object = {}
	
		this.for_every_object(filename, function(object, this_type_table)
			if all_objects_by_type[object.type] == nil then
				all_objects_by_type[object.type] = {}
			end
			
			table.insert(all_objects_by_type[object.type], object)
			type_tables_by_object[object] = this_type_table
		end)
		
		return all_objects_by_type, type_tables_by_object
	end,
	
	get_all_textures = function (filename)
		local this = tiled_map_loader
		local needed_textures = {}
		
		this.for_every_object(filename, function(object, this_type_table)
			local texture_name = this_type_table[this.texture_property_name]
				
			if texture_name ~= nil then
				needed_textures[texture_name] = true
			end
		end)
		
		return needed_textures
	end,
		
	load_world_properties = function (filename)
		local this = tiled_map_loader
		
		local world_information = {}
	
		this.for_every_object(filename, function(object, this_type_table)
			-- if type of this object matches with any requested world information string (e.g. PLAYER_POS, ENEMY_POS),
			-- then insert this object into world information table for this map
			if require(this.world_information_library)[object.type] == true then
				if world_information[object.type] == nil then
					world_information[object.type] = {}
				end
				
				table.insert(world_information[object.type], object)
			end
		end)
		
		return world_information
	end,

	basic_entity_table = function(object, this_type_table, out_polygons, out_rects)
		local this = tiled_map_loader
		local final_entity_table = {}
			
		local final_color = rgba(255, 255, 255, 255)
		
		if this_type_table.color ~= nil then
			final_color = this_type_table.color
		end
		
		-- begin processing the newly to be created entity
		local shape = object.shape
		local used_texture = textures_by_name[this_type_table.texture]
		
		local physics_body_type = 0
		
		if shape == "polygon" then
			physics_body_type = physics_info.POLYGON
			local new_polygon = simple_create_polygon (reversed((object.polygon)))
			map_uv_square(new_polygon, used_texture)
			set_polygon_color(new_polygon, final_color)
			
			final_entity_table.render = { model = new_polygon }
			table.insert(out_polygons, new_polygon)
		elseif shape == "rectangle" then
			physics_body_type = physics_info.RECT
			
			local rect_size = vec2(object.width, object.height)
			local new_rectangle = create_sprite { 
				image = used_texture,
				size = rect_size,
				color = final_color
			}
			
			final_entity_table.render = { model = new_rectangle }
			
			-- shift position by half of the rectangle size 
			object.pos = object.pos + rect_size / 2
			table.insert(out_rects, new_rectangle)
		else
			err ("shape type unsupported!")
		end
		
		if this_type_table.render_layer ~= nil then
			final_entity_table.render.layer = render_layers[this_type_table.render_layer]
		end
		
		final_entity_table.transform = {
			pos = object.pos
		}
		
		if this_type_table.scrolling_speed ~= nil then
			final_entity_table.chase = component_helpers.parallax_chase (tonumber(this_type_table.scrolling_speed), object.pos, this.world_camera_entity)
		end
		
		-- handle physical body request
		if this_type_table.entity_archetype.physics ~= nil then
			final_entity_table = archetyped(final_entity_table, { 
				physics = { 
					body_info = {
						shape_type = physics_body_type
					} 
				} 
			})
		end
		
		return archetyped(this_type_table.entity_archetype, final_entity_table)
	end,
	
	create_entities_from_map = function (filename)	
		local this = tiled_map_loader
		local err = this.error_callback
		
		local map_object = {
			all_entities = {
				named = {},
				unnamed = {}
			},
			
			all_polygons = {},
			all_sprites = {}
		}
			
		this.for_every_object(filename, function(object, this_type_table)
			-- do it only for non-property entities
			if require(this.world_information_library)[object.type] == nil then
				
				-- create the entity
				local new_entity = create_entity (this.basic_entity_table(object, this_type_table, map_object.all_polygons, map_object.all_sprites))
				
				-- and save it in map table
				if object.name == "" then 
					table.insert(map_object.all_entities.unnamed, new_entity)
				else
					if map_object.all_entities.named[object.name] ~= nil then
						err ("name conflict: " .. object.name)
					end
					
					map_object.all_entities.named[object.name] = new_entity
				end
			end				
		end)
		
		return map_object
	end
}


