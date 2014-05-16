-- Create a new class that inherits from a base class
--
function inherits_from(baseClass)

    -- The following lines are equivalent to the SimpleClass example:

    -- Create the table and metatable representing the class.
    local new_class = {}
    local class_mt = { __index = new_class }

    -- Note that this function uses class_mt as an upvalue, so every instance
    -- of the class will share the same metatable.
    --
    function new_class:create()
        local newinst = {}
        setmetatable( newinst, class_mt )
        return newinst
    end

    -- The following is the key to implementing inheritance:

    -- The __index member of the new class's metatable references the
    -- base class.  This implies that all methods of the base class will
    -- be exposed to the sub-class, and that the sub-class can override
    -- any of these methods.
    --
    if baseClass then
        setmetatable( new_class, { __index = baseClass } )
    end

    return new_class
end



function rewrite(component, entry, omit_properties)
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
	local lefttop = vec2(texcoords_to_map:get_vertex(0).pos.x, texcoords_to_map:get_vertex(0).pos.y)
	local bottomright = vec2(texcoords_to_map:get_vertex(0).pos.x, texcoords_to_map:get_vertex(0).pos.y)
	
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

global_sound_table = {}

function reversed(input_table)
	local out_table = {}
	
	for i = #input_table, 1, -1 do
		table.insert(out_table, input_table[i])
	end
	
	return out_table
end


function add_vals(target_vector, vals)
	for k, v in ipairs(vals) do
		target_vector:add(v)
	end
end

function orthographic_projection(left, right, bottom, top, near, far)
	local new_vec = float_vector()
	add_vals(new_vec, {
			2/(right-left), 0, 0, 0, 
			0, 2/(top-bottom), 0, 0,
			0, 0, -2/(far-near), 0, 
			-(right+left)/(right-left), -(top+bottom)/(top-bottom), -(far+near)/(far-near), 1
		}
	)
	
	return new_vec
end

