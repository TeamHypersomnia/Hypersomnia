entity_system = inherits_from ()

function entity_system:constructor()
	self.all_systems = {}
end

function entity_system:add_system(new_system)
	table.insert(all_systems, new_system)
end

function entity_system:for_all_matching_systems(component_set, callback)
	for i=1, #all_systems do
		local this_system = all_systems[i]
		local required_components = this_system:get_required_components()
		
		local matches = true
		for j=1, #required_components do
			if component_set[required_components[j]] == nil then 
				matches = false
				break
			end
		end
		
		if matches then
			callback(this_system)
		end
	end
	
end

function entity_system:add_entity(new_entity)
	self:for_all_matching_systems(new_entity, function(matching_system) matching_system:add(new_entity) end)
end

function entity_system:remove_entity(to_be_removed)
	self:for_all_matching_systems(new_entity, function(matching_system) matching_system:remove(new_entity) end)
end

processing_system = inherits_from ()

function processing_system:process_entities() end

function processing_system:constructor()
	self.targets = {}
end

function processing_system:get_required_components()
	return {}
end

function processing_system:add(new_entity) 
	table.insert(self.targets, new_entity)
end

function processing_system:remove(to_be_removed) 
	table.remove(self.targets, to_be_removed)
end

