-- Create a new class that inherits from a base class
--

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

function to_pixels(b2Vec2_)
	return vec2(b2Vec2_.x*METERS_TO_PIXELS, b2Vec2_.y*METERS_TO_PIXELS)
end

function to_meters(vec2_)
	return b2Vec2(vec2_.x*PIXELS_TO_METERS, vec2_.y*PIXELS_TO_METERS)
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

function add_roots_to_filenames(root_path, entries)
	local new_table = {}
	
	for k, v in pairs(entries) do
		new_table[k] = (root_path .. v)
	end
	
	return new_table
end

function get_all_files_in_directory(directory, add_roots)
	local relative_directory = directory
	directory = get_executable_path() .. "\\" .. relative_directory
	
    local i, t, popen = 0, {}, io.popen
    for filename in popen('dir "'..directory..'" /b'):lines() do
        i = i + 1
		
		if add_roots == true then
			t[i] = relative_directory .. "\\" .. filename
		else
			t[i] = filename
		end
    end
    return t
end

function tokenize_string(inputstr, sep)
    if sep == nil then sep = "%s" end
    local i, t = 1, {}
	
    for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
            t[i] = str
            i = i + 1
    end
	
    return t
end


function acquire_item_from_library(tokens, item_library)
	local current_array_level = item_library
	
	for i=1, #tokens do
		if current_array_level[tokens[i]] == nil then
			return current_array_level
		else
			current_array_level = current_array_level[tokens[i]]
		end
	end
	
	return current_array_level
end

function save_resource_in_item_library(filename, resource_object, item_library)
	local tokens = tokenize_string(filename, "_.")
	local current_array_level = item_library
	
	for i=1, #tokens do
		-- the last token is file extension
		if i ~= #tokens then
			
			-- the one before last should already point to the texture
			if i == (#tokens - 1) then
				current_array_level[tokens[i]] = resource_object
			else
				if current_array_level[tokens[i]] == nil then
					current_array_level[tokens[i]] = {}
				end
				
				current_array_level = current_array_level[tokens[i]]			
			end
		end
	end
end

function create_atlas_from_filenames(filename_entries, all_fonts)
	if not all_fonts then
		all_fonts = {}
	end
	
	local sprite_library = {}
	local sprite_object_library = {}
	
	local texture_by_filename = {}
	local out_atlas = atlas()
	collectgarbage("collect")
	
	-- save every texture object in item library to be used later
	for k, v in pairs(filename_entries) do
		local texture_object = texture(v, out_atlas)
		
		local sprite_object = create_sprite {
			image = texture_object
		}
		
		-- save for requests from map editor
		texture_by_filename[v] = texture_object
		
		-- tokenize filename to only get the filename and the extension
		local tokenized = tokenize_string(v, "\\/")
		
		-- the last token is just filename + extension
		save_resource_in_item_library(tokenized[#tokenized], texture_object, sprite_library)
		save_resource_in_item_library(tokenized[#tokenized], sprite_object, sprite_object_library)
	end
		
	local font_files = {}
	local font_by_name = {}
	
	local function get_font(filename, size, letters)
		local new_font_file = font_file()
		new_font_file:open(filename, size, letters)
		
		local new_font_object = font_instance()
		new_font_object:build(new_font_file)
		new_font_object:add_to_atlas(out_atlas)	
	
		table.insert(font_files, new_font_file)
		
		return new_font_object
	end
	
	for k, v in pairs(all_fonts) do
		font_by_name[k] = get_font(v.filename, v.size, v.letters)
	end
	
	out_atlas:build()
	out_atlas:nearest()
	
	return out_atlas, sprite_library, sprite_object_library, texture_by_filename, font_files, font_by_name
end

table.erase = function(self, element)
	for i=1, #self do
		if self[i] == element then
			table.remove(self, i)
			break
		end
	end
end

function bool2int(expr)
	if expr then return 1 else return 0 end
end


function copy_bitstream_for_reading(input_bs)
	local copy_bs = BitStream()
	copy_bs:assign(input_bs)
	return copy_bs
end

function set_rate(target, what_rate, updates_per_second)
	target[what_rate .. "_rate"] = updates_per_second
	target[what_rate .. "_interval_ms"] = 1000/updates_per_second
	target[what_rate .. "_timer"] = timer()
	
	target[what_rate .. "_ready"] = function(self)
		local result = self[what_rate .. "_timer"]:get_milliseconds() > self[what_rate .. "_interval_ms"]
		if result then self[what_rate .. "_reset"](self) end
		return result
	end
	
	target[what_rate .. "_reset"] = function(self)
		self[what_rate .. "_timer"]:reset()
	end
	
	target[what_rate .. "_time_remaining"] = function(self)
		return self[what_rate .. "_interval_ms"] - self[what_rate .. "_timer"]:get_milliseconds() 
	end
end

function setlsys(sys)
	debug_line_system = sys
end

function clearl()
	debug_line_system:clear_non_cleared_lines()
end

function debugl(col, pos, pos2)
	if pos2 == nil then 
		pos2 = vec2(pos)
		pos2.y = pos2.y + 10
	end
	
	debug_line_system:push_non_cleared_line(debug_line(pos, pos2, col))
end

function debuglb2(col, pos, pos2)
	if pos2 == nil then 
		debugl(col, vec2(pos.x*METERS_TO_PIXELS, pos.y*METERS_TO_PIXELS))
	else
		debugl(col, vec2(pos.x*METERS_TO_PIXELS, pos.y*METERS_TO_PIXELS), vec2(pos2.x*METERS_TO_PIXELS, pos2.y*METERS_TO_PIXELS))
	end
end

function clearlc(c)
	debug_line_system:clear_channel(c)
end

function debuglc(c, col, pos, pos2)
	if pos2 == nil then 
		pos2 = vec2(pos)
		pos2.y = pos2.y + 10
	end
	
	debug_line_system:push_line_channel(debug_line(pos, pos2, col), c)
end

function debuglcb2(c, col, pos, pos2)
	if pos2 == nil then 
		debugl(c, col, vec2(pos.x*METERS_TO_PIXELS, pos.y*METERS_TO_PIXELS))
	else
		debugl(c, col, vec2(pos.x*METERS_TO_PIXELS, pos.y*METERS_TO_PIXELS), vec2(pos2.x*METERS_TO_PIXELS, pos2.y*METERS_TO_PIXELS))
	end
end

array_shuffler = inherits_from()

function array_shuffler:constructor(array)
	self.array = table.shuffle(array)
	self.current_index = 1
end

function array_shuffler:next_value()
	if self.current_index > #self.array then
		self.current_index = 1
		self.array = table.shuffle(self.array)
	end

	local out = self.array[self.current_index]
	self.current_index = self.current_index + 1
	return out
end

expiration_timer = inherits_from()

function expiration_timer:constructor(expiration_ms)
	self.expiration_ms = expiration_ms
	self.timer_obj = timer()
end

function expiration_timer:expired()
	return self.timer_obj:get_milliseconds() > self.expiration_ms
end

function closest_to(target_pos)
	return function(a, b) return (a.pos - target_pos):length_sq() < (b.pos - target_pos):length_sq() end
end

function table.best(entries, better, it_method)
	if not entries or not next(entries) then return nil end
	
	local best_elem = entries[next(entries)]
	
	if not better then
		better = function (a, b) return a < b end 
	end
	
	if it_method then
		for k, v in pairs(entries) do
			if better(v, best_elem) then
				best_elem = v 
			end
		end
	else
		for i=2, #entries do
			if better(entries[i], best_elem) then
				best_elem = entries[i] 
			end
		end
	end
	
	return best_elem
end