entity_system = inherits_from ()

function entity_system:constructor(deletes_caller)
	self.deletes_caller = deletes_caller
	self.all_systems = {}
	self.messages = {}
	
	self.to_be_removed = {}
	
	self.cpp_entity = cpp_entity_system:create()
	self:register_systems( { ["cpp_entity"] = self.cpp_entity } )
end

function entity_system:register_messages(msgs)
	for k, v in pairs(msgs) do
		if self.messages[v] == nil then self.messages[v] = {} end
	end
end

function entity_system:flush_messages()
	for k, v in pairs(self.messages) do
		self.messages[k] = {}
	end
end

function entity_system:post(name, msg_table)
	self:post_table(name, _G[name]:create(msg_table))
end

function entity_system:post_table(name, msg_table)
	table.insert(self.messages[name], msg_table)
end

function entity_system:register_systems(new_systems)
	for k, v in pairs(new_systems) do
		v.owner_entity_system = self
		self.all_systems[k] = v
	end
end

function entity_system:post_remove(removed_entity)
	self.to_be_removed[removed_entity] = true
end

function entity_system:handle_removed_entities()
	for k, v in pairs (self.to_be_removed) do
		self:remove_entity(k)
	end
	
	self.to_be_removed = {}
	
	self.deletes_caller()
end

function entity_system:for_all_matching_systems(component_set, callback)
	for k, v in pairs(self.all_systems) do
		local required_components = v:get_required_components()
		
		if required_components ~= nil then
			local matches = true
			for j=1, #required_components do
				if component_set[required_components[j]] == nil then 
					matches = false
					break
				end
			end
			
			if matches then
				callback(v)
			end
		end
	end
	
end

function entity_system:add_entity(new_entity)
	self:for_all_matching_systems(new_entity, function(matching_system) matching_system:add_entity(new_entity) end)
	
	return new_entity
end

function entity_system:remove_entity(to_be_removed)
	self:for_all_matching_systems(to_be_removed, function(matching_system) matching_system:remove_entity(to_be_removed) end)
end

processing_system = inherits_from ()

function processing_system:constructor()
	self.targets = {}
end

function processing_system:get_required_components()
	return nil
end

function processing_system:add_entity(new_entity) 
	table.insert(self.targets, new_entity)
end

function processing_system:remove_entity(to_be_removed) 
	table.erase(self.targets, to_be_removed)
end


-- convenience table
components = {}

components.create_components = function(entry)
	local output = {}
	
	for k, v in pairs(entry) do
		if type(v) == "table" then
			output[k] = components[k]:create(v)
		else
			output[k] = v
		end
	end	
	
	return output
end


-- convenience processing_system for entities from cpp framework
cpp_entity_system = inherits_from (processing_system)

function cpp_entity_system:get_required_components()
	return { "cpp_entity" }
end

function cpp_entity_system:add_entity(new_entity)
	-- don't store anything
	new_entity.cpp_entity.script = new_entity
end

function cpp_entity_system:remove_entity(removed_entity)
	-- get world object owning that entity and delete it
	local owner_world = removed_entity.cpp_entity.owner_world
	owner_world:post_message(destroy_message(removed_entity.cpp_entity, nil))
end





