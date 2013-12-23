function set_components_from_entry(entity, entry, entities_lookup)
	if entry.transform ~= nil then
		local transform = entity:add(transform_component())
		if entry.transform.pos ~= nil then transform.current.pos = entry.transform.pos end
		if entry.transform.rotation ~= nil then transform.current.rotation = entry.transform.rotation end
	end	
	
	if entry.children ~= nil then
		local children = entity:add(children_component())
		
		for i, child in pairs(entry.children) do
			children:add(entity_ptr(ptr_lookup(child, entities_lookup)))
		end
	end
	
	if entry.input ~= nil then
		local input = entity:add(input_component())
		
		for i, intent in pairs(entry.input) do
			input:add(intent)
		end
	end
	
	if entry.movement ~= nil then
		local movement = entity:add(movement_component())
		
		if entry.movement.receivers ~= nil then
			for i, receiver in pairs(entry.movement.receivers) do
				movement:add_animation_receiver(entity_ptr(ptr_lookup(receiver.target, entities_lookup)), receiver.stop_at_zero_movement)
			end
		end
		
		rewrite(movement, entry.movement)
	end
	
	function def(module_name, property_name, omit_properties) 
		if(entry[property_name] ~= nil) then
			local component = entity:add(module_name())
			rewrite(component, entry[property_name], omit_properties)
		end
	end
	
	function def_ptr(module_name, property_name, ptr_variables, omit_properties) 
		if(entry[property_name] ~= nil) then
			local component = entity:add(module_name())
			
			omit_properties = omit_properties or {}
			for k, v in pairs(ptr_variables) do
				omit_properties[k] = v
			end
			
			rewrite(component, entry[property_name], omit_properties)
			rewrite_ptr(component, entry[property_name], ptr_variables, entities_lookup)
		end
	end
	
	def		(render_component, 'render')
	def		(animate_component, 'animate')
	def_ptr	(camera_component, 'camera', { crosshair = true, player = true})
	def_ptr	(chase_component, 'chase', { target = true })
	def		(crosshair_component, 'crosshair')
	def_ptr (damage_component, 'damage', { sender = true })
	def_ptr (gun_component, 'gun', { target_camera_to_shake = true }, { bullet_body = true, bullet_render = true })
	def		(health_component, 'health', { corpse_body = true, corpse_render = true } )
	def_ptr (lookat_component, 'lookat', { target = true })
	def		(particle_emitter_component, 'particle_emitter')
	def		(scriptable_component, 'scriptable')
	def		(visibility_component, 'visibility', {}, { visibility_layers = true })
	def		(pathfinding_component, 'pathfinding')
	def		(steering_component, 'steering')
	def		(behaviour_tree_component, 'behaviour_tree')
	
	if entry.visibility ~= nil then
		local visibility = entity.visibility
		
		for key, values in pairs (entry.visibility.visibility_layers) do
			local my_visibility = visibility_layer()
			recursive_write(my_visibility, values)
			visibility:add_layer(key, my_visibility)
		end
	end
	
	if entry.physics ~= nil then
		local my_body_data = physics_info()
		entry.physics.body_info = entry.physics.body_info or {}
		
		if entry.physics.body_info.shape_type ~= nil and entry.physics.body_info.shape_type == physics_info.RECT then
			if entry.physics.body_info.rect_size == nil then
				entry.physics.body_info.rect_size = entry.render.model.size
			end
		end
		
		set_physics_info(my_body_data, entry.physics.body_info)
		
		create_physics_component(my_body_data, entity, entry.physics.body_type)
	end

	if entry.gun ~= nil then
		local gun = entity.gun
		--print(inspect(entry.gun))
		entry.gun.bullet_body = entry.gun.bullet_body or {}
		entry.gun.bullet_render = entry.gun.bullet_render or {} 
		
		set_physics_info(gun.bullet_body, entry.gun.bullet_body)
		rewrite(gun.bullet_render, entry.gun.bullet_render)
	end
	
	if entry.health ~= nil then
		local health = entity.health
		set_physics_info(health.corpse_body, entry.health.corpse_body)
		rewrite(health.corpse_render, entry.health.corpse_render)
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
		set_components_from_entry(entry, entries[name], entities_lookup)
	end
	
	return entities_lookup
end