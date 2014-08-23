-- enables modules to be read/written correctly in order
protocol.module_mappings = {}

protocol.replication_tables = {}

protocol.replication_tables.register = function (table_name, entry)
	table.insert(protocol.module_mappings, table_name)
	table.sort(protocol.module_mappings)
	
	protocol.replication_tables[table_name] = { 
		properties = create_replication_properties (entry.properties), 
		init_only_fields = create_replication_properties (entry.init_only_fields), 
		updaters = entry.optional_updaters
	}
end

function create_replica(modules)
	local new_object = {}
	
	for i=1, #modules do
		new_object[modules[i]] = replication_module:create(protocol.replication_tables[modules[i]])
	end
	
	return new_object
end


local NUM_FIELD_PROPERTIES = 3

function create_replication_properties(entry)
	local output = {}
	if entry == nil then return {} end
	
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
		
		output[#output+1] = new_var
	end
	
	return output
end

protocol.diff_operators = {
	-- accept divergences maximum of 0.1 pixels
	["b2Vec2"] = function (a, b) return b2Vec2(a.x-b.x, a.y-b.y):LengthSquared() > (0.5 * PIXELS_TO_METERS)*(0.5 * PIXELS_TO_METERS) end,
	["Vec2"] =   function (a, b) return (a-b):length_sq() > 0.5*0.5 end,
	["Float"] =   function (a, b) return math.abs(a-b) > 0.01 end
}

replication_module = inherits_from ()

function replication_module:constructor(replication_table)
	self.replication_table = replication_table
	self.has_variable_changed = {}
	self.FIELDS_READ = {}
end

function replication_module:write_initial_state(object, output_bs)
	-- this time, replicate as we go
	local properties = self.replication_table.init_only_fields
	
	for i=1, #properties do
		local field = properties[i]
		
		local real_gameobject_value = field.replication_func(object)
				
		output_bs:name_property(field.name)
		protocol.write_var(field.type, real_gameobject_value, output_bs)
	end
end


function replication_module:read_initial_state(object, input_bs)
	local properties = self.replication_table.init_only_fields
	
	for i=1, #properties do
		local field = properties[i]
		
		input_bs:name_property(field.name)
		self[field.name] = protocol.read_var(field.type, input_bs)
		self:call_updater(object, field)
	end
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


function replication_module:get_all_marked()
	local out = {}
	
	for i=1, #self.replication_table.properties do
		out[i] = true
	end
	
	return out
end

function replication_module:ack_flags(flags_table, ack_sequence)
	for key, field_sequence in pairs(flags_table) do
		-- if field_sequence is a boolean,
		-- the flag was not yet transmitted because of differing replication rates,
		-- thus it has to stay marked for that client for later transmission
		if type(field_sequence) == "number" and field_sequence <= ack_sequence then
			flags_table[key] = nil
		end
	end
end

function replication_module:mark_out_of_date_fields(flags_table)
	-- create dirty flags requested by the replica (self)
	for key in pairs(self.has_variable_changed) do
		flags_table[key] = true
	end
end

replication_module.request_fields_transmission = function(flags_table, next_sequence)
	for key, field in pairs(flags_table) do
		if type(field) == "boolean" then
			flags_table[key] = next_sequence
		end
	end
end

function replication_module:write_state(which_fields, output_bs)
	local properties = self.replication_table.properties
	if #properties <= 0 then return false end
	
	local module_transmitted = false
	
	for k, v in pairs(which_fields) do
		if type(v) == "number" then 
			module_transmitted = true 
			break 
		end
	end
	
	output_bs:name_property("has module changed")
	output_bs:WriteBit(module_transmitted)
	
	if module_transmitted then
		for i=1, #properties do
			local field = properties[i]
			
			output_bs:name_property("has field " .. field.name .. " changed")
	
			if type(which_fields[i]) == "number" then
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

function replication_module:call_updater(object, field)
	local updaters = self.replication_table.updaters
			
	if updaters ~= nil then
		if updaters[field.name] ~= nil then
			updaters[field.name](object, self[field.name], self)
		elseif updaters.GENERIC_UPDATER ~= nil then
			updaters.GENERIC_UPDATER(object, field.name, self[field.name], self)
		end
	end
end

function replication_module:read_state(object, input_bs)
	local properties = self.replication_table.properties
	if #properties <= 0 then return false end
	
	input_bs:name_property("has module changed")
	local module_changed = input_bs:ReadBit()
	
	self.FIELDS_READ = {}
	
	if module_changed then
		
		for i=1, #properties do
			local field = properties[i]
			
			input_bs:name_property("has field " .. field.name .. " changed")
			
			
			if input_bs:ReadBit() then
				self.FIELDS_READ[field.name] = true
				self[field.name] = protocol.read_var(field.type, input_bs)
				self:call_updater(object, field)
			end
		end
	end
end