function set_components_from_entry(_entity, entry, entities_lookup)
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
		
		rewrite(movement, entry.movement)
	end
	
	function def(module_name, property_name, ptr_variables) 
		if(entry[property_name] ~= nil) then
			local component = _entity:add(module_name())
			rewrite(component, entry[property_name], ptr_variables)
			rewrite_ptr(component, entry[property_name], ptr_variables, entities_lookup)
		end
	end
	
	def(render_component, 'render')
	def(animate_component, 'animate')
	def(camera_component, 'camera', { crosshair = true, player = true})
	def(chase_component, 'chase', { target = true })
	def(crosshair_component, 'crosshair')
	def(damage_component, 'damage', { sender = true })
	def(gun_component, 'damage', { target_camera_to_shake = true })
	def(health_component, 'health')
	def(lookat_component, 'lookat', { target = true })
	def(particle_emitter_component, 'particle_emitter')
	
	if entry.physics ~= nil then
		local filter = b2Filter()
		rewrite(filter, entry.physics.filter)
			
		create_physics_component(_entity, filter, entry.physics.body_type)
	end
end

function create_entity(entry)
	local new_entity = world:create_entity()
	set_components_from_entry(new_entity, entry, {})
	return new_entity
end

function create_entity_group(entries)
	local entities_lookup = {}
	
	for name, entry in pairs(entries) do
		entities_lookup[name] = world:create_entity() 
	end
	
	for name, entry in pairs(entities_lookup) do
		set_components_from_entry(entry, final_entries[name], entities_lookup)
	end
end