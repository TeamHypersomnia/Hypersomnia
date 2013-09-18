function rewrite(component, entry, properties) 
	for i, property in pairs(properties) do
		if entry[property] ~= nil then
			component[property] = entry[property]
		end
	end
end

function rewrite_ptr(component, entry, properties, entities_lookup) 
	for i, property in pairs(properties) do
		if entry[property] ~= nil then
			if type(entry[property]) == "string" then
				if(entities_lookup[entry[property]] ~= nil) then
					component[property]:set(entities_lookup[entry[property]])
				end  
			else
				component[property]:set(entry[property])
			end
		end
	end
end

function recursive_write(entries, final_entries)
	for key, entry in pairs(entries) do
		if type(entry) == "table" then
			if final_entries[key] == nil then final_entries[key] = {} end
			recursive_write(entry, final_entries[key])
		else
			final_entries[key] = entry
		end
	end
end

function entries_from_archetypes(entries, final_entries)
	if entries.archetype ~= nil then
		entries_from_archetypes(entries.archetype, final_entries)
	end
	
	recursive_write(entries, final_entries)
end

function set_components_from_entry(_entity, _entry, entities_lookup)
	local entry = {}
	entries_from_archetypes(_entry, entry)
	
	if entry.transform ~= nil then
		local transform = _entity:add(transform_component())
		if entry.transform.pos ~= nil then transform.current.pos = entry.transform.pos end
		if entry.transform.rotation ~= nil then transform.current.rotation = entry.transform.rotation end
	end	
	
	if entry.children ~= nil then
		local children = _entity:add(children_component())
		
		for i, child in pairs(entry.children) do
			children:add(child)
		end
	end
	
	if entry.input ~= nil then
		local input = _entity:add(input_component())
		
		for i, intent in pairs(entry.input) do
			input:add(intent)
		end
	end
	
	if entry.movement ~= nil then
		local movement = _entity:add(movement_component())
		
		if entry.movement.receivers ~= nil then
			for i, receiver in pairs(entry.movement.receivers) do
				movement:add_animation_receiver(receiver.target, receiver.stop_at_zero_movement)
			end
		end
		
		rewrite(movement, entry.movement, {'acceleration', 'max_speed'})
	end
	
	if entry.physics ~= nil then
		local filter = b2Filter()
		rewrite(filter, entry.physics.filter, {'categoryBits', 'maskBits', 'groupIndex'})
			
		create_physics_component(_entity, filter, entry.physics.body_type)
	end
	
	function def(module_name, property_name, variables, ptr_variables) 
		if(entry[property_name] ~= nil) then
			local component = _entity:add(module_name())
			rewrite(component, entry[property_name], variables)
			rewrite_ptr(component, entry[property_name], ptr_variables, entities_lookup)
		end
	end
	
	def(render_component, 'render', {'layer', 'image', 'mask'}, {})
	def(animate_component, 'animate', {'available_animations'}, {})
	def(camera_component, 'camera', {'enabled', 'layer', 'mask', 'screen_rect', 'ortho',
		 'enable_smoothing', 'smoothing_average_factor', 'averages_per_sec', 'crosshair', 'player', 
		 'orbit_mode', 'max_look_expand',
		'angled_look_length' }, {})
	
	def(chase_component, 'chase', {'type', 'offset', 'rotation_orbit_offset', 'rotation_offset', 'relative', 'chase_rotation', 'track_origin'}, {'target'})
	def(crosshair_component, 'crosshair', {'should_blink', 'sensitivity'}, {})
	def(damage_component, 'damage', {'amount', 'starting_point', 'max_distance'}, {'sender'})
	def(gun_component, 'damage', {'info', 'current_rounds', 'reloading'}, {'target_camera_to_shake'})
	def(health_component, 'health', {'info', 'hp', 'dead'}, {})
	def(lookat_component, 'lookat', {'look_mode'}, {'target'})
	def(particle_emitter_component, 'particle_emitter', {'available_particle_effects'})
end

function create_entity_from_entry(entry)
	local new_entity = world:create_entity()
	set_components_from_entry(new_entity, entry, {})
	return new_entity
end

function create_entity_group(entries)
	local final_entries = {}
	entries_from_archetypes(entries, final_entries)
	
	local entities_lookup = {}
	
	for name, entry in pairs(final_entries) do
		if name ~= "archetype" then entities_lookup[name] = world:create_entity() end 
	end
	
	for name, entry in pairs(entities_lookup) do
		set_components_from_entry(entry, final_entries[name], entities_lookup)
	end
end