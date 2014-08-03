-- enables modules to be read/written correctly in order
protocol.module_mappings = {
	--"client_info",
	"movement",
	"crosshair",
	"health"
}

protocol.replication_tables = {}


local NUM_FIELD_PROPERTIES = 3

function create_replication_table(entry, optional_updaters)
	local output = { properties = {} }
	local k = NUM_FIELD_PROPERTIES
	
	for i=1, (#entry/k) do
		local type_name = 	 	   entry[i*k-k+1]
		local name = 			   entry[i*k-k+2]
		local replication_func =   entry[i*k-k+3]
		
		local new_var = {
			["type"] = type_name,
			["name"] = name,
			["replication_func"] = replication_func
		}
		
		if type(update_object_func) == "function" then
			new_var.update_object_func = update_object_func
		end
		
		output.properties[#output.properties+1] = new_var
	end
		
	output.updaters = optional_updaters
	
	return output
end


protocol.diff_operators = {
	-- accept divergences maximum of 0.1 pixels
	["b2Vec2"] = function (a, b) return b2Vec2(a.x-b.x, a.y-b.y):LengthSquared() > (0.5 * PIXELS_TO_METERS)*(0.5 * PIXELS_TO_METERS) end,
	["Vec2"] =   function (a, b) return (a-b):length_sq() > 0.5*0.5 end
}

replication_module = inherits_from ()

function replication_module:constructor(replication_table)
	self.replication_table = replication_table
end

function replication_module:replicate(object)
	-- zero-out the table every update
	self.has_variable_changed = {}
	
	local properties = self.replication_table.properties
	
	for i=1, #properties do
		local field = properties[i]
		
		local real_gameobject_value = field.replication_func(object)
		local diff_operator = protocol.diff_operators[field.type]
		
		if self[i] == nil or 
			(diff_operator ~= nil and diff_operator(self[i], real_gameobject_value)) or
			(diff_operator == nil and self[i] ~= real_gameobject_value)
			then
			
			-- variable is a subject to rentransmission if it changed while being within somebody's AoI
			-- objects outside AoI are dropped
			self.has_variable_changed[i] = true
			self[i] = real_gameobject_value
		end
	end
end


function replication_module:get_all_marked(next_sequence)
	local out = {}
	
	for i=1, #self.replication_table.properties do
		out[i] = next_sequence
	end
	
	return out
end

function replication_module:update_flags(flags_table, next_sequence, ack_sequence)
	-- firstly mark acked
	for key, field_sequence in pairs(flags_table) do
		if field_sequence <= ack_sequence then
			flags_table[key] = nil
		end
	end
	
	-- now create dirty flags requested by the replica (self)
	for key, field in pairs(self.has_variable_changed) do
		flags_table[key] = next_sequence
	end
end

function replication_module:write_state(which_fields, output_bs)
	local module_changed = next(which_fields) ~= nil
	
	output_bs:name_property("has module changed")
	output_bs:WriteBit(module_changed)
	
	local properties = self.replication_table.properties
	
	if module_changed then
		for i=1, #properties do
			local field = properties[i]
			
			output_bs:name_property("has field " .. field.name .. " changed")
	
			if which_fields[i] ~= nil then
				output_bs:WriteBit(true)
				
				output_bs:name_property(field.name)
				protocol.write_var(field.type, self[i], output_bs)
			else
				output_bs:WriteBit(false)
			end
		end
		
		return true
	end
	
	return false
end

function replication_module:read_state(object, input_bs)
	input_bs:name_property("has module changed")
	local module_changed = input_bs:ReadBit()
	
	if module_changed then
		local properties = self.replication_table.properties
		
		for i=1, #properties do
			local field = properties[i]
			
			input_bs:name_property("has field " .. field.name .. " changed")
			
			if input_bs:ReadBit() then
				self[field.name] = protocol.read_var(field.type, input_bs)
				
				local updaters = self.replication_table.updaters
				
				if updaters ~= nil then
					if updaters[field.name] ~= nil then
						updaters[field.name](object, self[field.name], self)
					elseif updaters.GENERIC_UPDATER ~= nil then
						updaters.GENERIC_UPDATER(object, field.name, self[field.name], self)
					end
				end
			end
		end
	end
end