function rewrite(component, entry, omit_properties)
	--print(inspect(entry))
	if omit_properties == nil then
		for key, val in pairs(entry) do
			component[key] = val
		end
	else
		for key, val in pairs(entry) do
			if omit_properties[key] == nil then
				component[key] = val
			end
		end
	end
end

function ptr_lookup(entry, entities_lookup) 
	if type(entry) == "string" then
		return entities_lookup[entry]
	else
		return entry
	end
end

function rewrite_ptr(component, entry, properties, entities_lookup)
	if properties == nil then return end
	
	for key, val in pairs(properties) do
		component[key]:set(ptr_lookup(entry[key], entities_lookup))
	end
end

function recursive_write(final_entries, entries, omit_names)
	omit_names = omit_names or {}
	
	for key, entry in pairs(entries) do
		if omit_names[key] == nil then
			if type(entry) == "table" then
				if final_entries[key] == nil then final_entries[key] = {} end
				recursive_write(final_entries[key], entry)
			else
				final_entries[key] = entry
			end
		end
	end
end

function entries_from_archetypes(archetype, entries, final_entries)
	recursive_write(final_entries, archetype)
	recursive_write(final_entries, entries)
end

function archetyped(archetype, entries)
	local final_entries = {}
	entries_from_archetypes(archetype, entries, final_entries)
	return final_entries
end

function map_uv_square(texcoords_to_map, texture_to_map)
	local lefttop = vec2()
	local bottomright = vec2()
	
	for i = 0, texcoords_to_map:get_vertex_count()-1 do
		local v = texcoords_to_map:get_vertex(i).pos
		if v.x < lefttop.x then lefttop.x = v.x end
		if v.y < lefttop.y then lefttop.y = v.y end
		if v.x > bottomright.x then bottomright.x = v.x end
		if v.y > bottomright.y then bottomright.y = v.y end
	end
	
	for i = 0, texcoords_to_map:get_vertex_count()-1 do
		local v = texcoords_to_map:get_vertex(i)
		v:set_texcoord (vec2(
		(v.pos.x - lefttop.x) / (bottomright.x-lefttop.x),
		(v.pos.y - lefttop.y) / (bottomright.y-lefttop.y)
		), texture_to_map)
	end
end