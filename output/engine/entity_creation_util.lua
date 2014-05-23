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
				movement:add_animation_receiver(ptr_lookup(receiver.target, entities_lookup), receiver.stop_at_zero_movement)
			end
		end
		
		rewrite(movement, entry.movement, { ground_filter = true } )
		if entry.movement.ground_filter ~= nil then 
			recursive_write(movement.ground_filter, entry.movement.ground_filter) 
		end
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
			
			if omit_properties == nil then omit_properties = {} end
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
	def_ptr	(chase_component, 'chase', { }, { target = true } )
	def		(crosshair_component, 'crosshair')
	def_ptr (damage_component, 'damage', { sender = true })
	def_ptr (gun_component, 'gun', { target_camera_to_shake = true }, { bullet_body = true, bullet_render = true })
	def_ptr (lookat_component, 'lookat', { target = true })
	def		(particle_emitter_component, 'particle_emitter')
	def		(scriptable_component, 'scriptable')
	def		(visibility_component, 'visibility', {}, { visibility_layers = true })
	def		(pathfinding_component, 'pathfinding')
	def		(steering_component, 'steering')
	def		(behaviour_tree_component, 'behaviour_tree',  {}, { trees = true })
	
	if entry.chase ~= nil then
		if entry.chase.target ~= nil then 
			entity.chase:set_target(ptr_lookup(entry.chase.target, entities_lookup))
		end
	end
	
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
		if entry.physics.body_info == nil then entry.physics.body_info = {} end
		
		if entry.physics.body_info.shape_type == physics_info.RECT then
			if entry.physics.body_info.rect_size == nil then
				entry.physics.body_info.rect_size = entry.render.model.size
			end
		end
		
		if entry.physics.body_info.shape_type == physics_info.POLYGON then
			if entry.physics.body_info.vertices == nil then
				entry.physics.body_info.vertices = entry.render.model
			end
		end
		
		set_physics_info(my_body_data, entry.physics.body_info)
		
		create_physics_component(my_body_data, entity, entry.physics.body_type)
	end

	if entry.gun ~= nil then
		local gun = entity.gun
		--print(inspect(entry.gun))
		if entry.gun.bullet_body == nil then entry.gun.bullet_body = {} end
		if entry.gun.bullet_render == nil then entry.gun.bullet_render = {} end
		
		set_physics_info(gun.bullet_body, entry.gun.bullet_body)
		rewrite(gun.bullet_render, entry.gun.bullet_render)
	end
	
	if entry.behaviour_tree ~= nil then
		local behaviour_tree = entity.behaviour_tree
		
		for k, v in ipairs(entry.behaviour_tree.trees) do
			behaviour_tree:add_tree(v)
		end
	end
end

function world_class:create_entity(entry)
	local enabled = true
	
	if entry.enabled ~= nil then
		enabled = entry.enabled
	end
	
	local new_entity = self.world:create_entity()
	set_components_from_entry(new_entity, entry, {})
	
	if not enabled then 
		new_entity:disable()
	end
	
	return new_entity
end

function world_class:flush_dead_entities_in_group_by_entity()
	for k, v in pairs(self.group_by_entity) do
		if not k:exists() then
			self.group_by_entity[k] = nil
		end
	end
end

function world_class:get_group_by_entity(entity_entry)
	self:flush_dead_entities_in_group_by_entity()
	
	for k, v in pairs(self.group_by_entity) do
		if k:get() == entity_entry then
			return v
		end
	end
	
	return nil
end

function world_class:create_entity_group(entries)
	self:flush_dead_entities_in_group_by_entity()
	
	local entities_lookup = {}
	
	for name, entry in pairs(entries) do
		local new_entity_ptr = entity_ptr()
		local new_entity = self.world:create_entity()
		new_entity_ptr:set(new_entity)
		
		entities_lookup[name] = new_entity
		self.group_by_entity[new_entity_ptr] = entities_lookup
	end
	
	for name, entry in pairs(entities_lookup) do
		set_components_from_entry(entry, entries[name], entities_lookup)
	end
	
	return entities_lookup
end


function world_class:ptr_create_entity(entry)
	local result = self:create_entity(entry)
	local my_new_ptr = entity_ptr()
	my_new_ptr:set(result)
	return my_new_ptr
end

function world_class:ptr_create_entity_group(entries)
	local results = self:create_entity_group(entries)
	
	for name, entry in pairs(results) do
		local my_new_ptr = entity_ptr()
		my_new_ptr:set(entry)
		results[name] = my_new_ptr
	end
	
	return results
end

component_helpers = {}
component_helpers.parallax_chase = function(_scrolling_speed, init_pos, camera_entity)
	return {
			scrolling_speed = _scrolling_speed,
			reference_position = init_pos,
			target_reference_position = camera_entity.transform.current.pos,
			
			chase_type = chase_component.PARALLAX,
			target = camera_entity,
			subscribe_to_previous = true
		}
end

