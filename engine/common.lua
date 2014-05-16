-- Create a new class that inherits from a base class
--

function table.shuffle(array)
    local n = #array
    for i = n, 2, -1 do
        local j = randval_i(1, i)
        array[i], array[j] = array[j], array[i]
    end
    return array
end

function coroutine.wait_routine(my_timer, ms_wait, loop_func, constant_delta)
	if constant_delta == nil then constant_delta = false end

	local accumulated_time = 0
	local delta_multiplier = 1
	
	while true do
		local extracted_ms = my_timer:extract_milliseconds()
		if not constant_delta then extracted_ms = extracted_ms * delta_multiplier end
		
		accumulated_time = accumulated_time + extracted_ms
	
		if loop_func ~= nil then
			loop_func(accumulated_time)
		end
			
		local new_multiplier = coroutine.yield()
		
		if new_multiplier ~= nil then delta_multiplier = new_multiplier end	
			
		if accumulated_time >= ms_wait then
			break
		end
	end
end

function coroutine.stepped_wait(ms_wait, loop_func, constant_delta)
	coroutine.wait_routine(stepped_timer(physics_system), ms_wait, loop_func, constant_delta)
end

function coroutine.wait(ms_wait, loop_func, constant_delta)
	coroutine.wait_routine(timer(), ms_wait, loop_func, constant_delta)
end

function to_vec2(b2Vec2_)
	return vec2(b2Vec2_.x, b2Vec2_.y)
end

function inherits_from(baseClass)

    -- The following lines are equivalent to the SimpleClass example:

    -- Create the table and metatable representing the class.
    local new_class = {}
    local class_mt = { __index = new_class }

    -- Note that this function uses class_mt as an upvalue, so every instance
    -- of the class will share the same metatable.
    --
    function new_class:create(...)
        local newinst = {}
        setmetatable( newinst, class_mt )
		
		newinst:constructor(...)
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
	if omit_names == nil then omit_names = {} end
	
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

function reversed(input_table)
	local out_table = {}
	
	for i = #input_table, 1, -1 do
		table.insert(out_table, input_table[i])
	end
	
	return out_table
end


function add_vals(target_vector, vals)
	for i=1, #vals do
		target_vector:add(vals[i])
	end
end


function vector_to_table(source_vector)
	local output = {}
	local vec_size = source_vector:size()
	
	if vec_size < 1 then return {} end
	
	for i=0, vec_size-1 do
		table.insert(output, source_vector:at(i))
	end
	
	return output
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

function to_vec2_table(xytable)
	local newtable = {}
	
	for k, v in pairs(xytable) do
		newtable[k] = vec2(v.x, v.y)
	end
	
	return newtable
end

function table.concatenate(all_tables)
	local sum_of_all = {}
	
	if all_tables ~= nil then
		for	index, source_table in pairs(all_tables) do
			for key, val in ipairs(source_table) do
				table.insert(sum_of_all, val)
			end
		end
	end
	
	return sum_of_all
end

function spairs(t, order)
    -- collect the keys
    local keys = {}
    for k in pairs(t) do keys[#keys+1] = k end

    -- if order function given, sort by it by passing the table and keys a, b,
    -- otherwise just sort the keys 
    if order then
        table.sort(keys, function(a,b) return order(t, a, b) end)
    else
        table.sort(keys)
    end

    -- return the iterator function
    local i = 0
    return function()
        i = i + 1
        if keys[i] then
            return keys[i], t[keys[i]]
        end
    end
end


function coroutine.get_value_variator(args)  
	return coroutine.wrap(function() 
		local last_value = 0
			
		while true do
			if randval(0, 1) < args.wait_probability then
				coroutine.wait(randval(args.min_wait_ms, args.max_wait_ms), nil, false)
			end
		
			local transition_duration = randval(args.min_transition_ms, args.max_transition_ms)
			local target_value = randval(args.min_value, args.max_value)
			if args.value_additive then target_value = target_value + last_value end
			
			local my_val_animator = value_animator(last_value, target_value, transition_duration)
			
			if randval(0, 1) > 0.5 then
				my_val_animator:set_exponential()
			else
				my_val_animator:set_linear()
			end
			
			coroutine.wait(transition_duration, function()
				last_value = my_val_animator:get_animated()
				args.callback(last_value)
			end, args.constant_transition_delta)
		end
	end)
end