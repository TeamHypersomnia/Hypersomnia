function create(module, entries)
	local new_instance = module()
	for k, v in pairs(entries) do
		new_instance[k] = entries[k]
	end
	
	return new_instance
end

function create_options(entries, output)
	if output == nil then output = _G end
	for k, v in pairs(entries) do
		output[v] = bitflag(k-1)
	end
	
	return entries
end

function create_inverse_enum(entries)
	local new_table = {}
	
	for k, v in pairs(entries) do
		new_table[v] = 1000000000-k
	end
	
	return new_table
end

function create_sprite(entries)
	local my_sprite = sprite()
	rewrite(my_sprite, entries, {image = true})
	
	if entries.image == nil then
		my_sprite.image = nil
	else
		my_sprite.image = entries.image.tex
	end
	
	if entries.size == nil then 
		my_sprite:update_size()
	end
	
	if entries.size_multiplier ~= nil then
		my_sprite.size = vec2(my_sprite.size.x * entries.size_multiplier.x, my_sprite.size.y * entries.size_multiplier.y)
	end	
	
	return my_sprite
end

function create_polygon(entries) 
	local my_polygon = polygon()
	local my_concave = drawable_concave()
	
	for k, vert in pairs(entries) do
		my_concave:add_vertex(vertex(vert.pos, vert.texcoord, vert.color, vert.image.tex))
	end
	
	my_polygon:add_concave(my_concave)
	
	return my_polygon
end


function gen_circle_vertices(radius, num_verts)
	local results = {}
	
	for i = 1,num_verts do
		results[i] = (vec2.from_degrees((360 / num_verts) * i))
		results[i]:set_length(radius)
	end

	return results
end

function simple_create_polygon(entries) 
	local my_polygon = polygon()
	local my_concave = drawable_concave()
	
	for i=1, #entries do
		my_concave:add_vertex(vertex(entries[i]))
	end
	
	my_polygon:add_concave(my_concave)
	
	return my_polygon
end

function create_animation(entries) 
	local my_animation = animation()
	rewrite(my_animation, entries)
	
	for i = 1, #(entries.frames) do
		-- shortcut
		local m = entries.frames[i].model
		if m == nil then
			my_animation:add_frame(create_sprite { image = nil }, entries.frames[i].duration_ms, entries.frames[i].callback, entries.frames[i].callback_out)
		elseif type(m) == "userdata" then
			my_animation:add_frame(create_sprite{ image = m } , entries.frames[i].duration_ms, entries.frames[i].callback, entries.frames[i].callback_out)
		else
			my_animation:add_frame(create_sprite(m), entries.frames[i].duration_ms, entries.frames[i].callback, entries.frames[i].callback_out)
		end
	end
	
	return my_animation
end

function create_animation_set(entries) 
	local my_animation_set = animate_info()
	
	for k,v in pairs(entries.animations) do
		my_animation_set:add(v.event, v.animation_response)
	end
	
	return my_animation_set
end

function create_input_context(entries) 
	local my_input_context = input_context()
	
	for k, v in pairs(entries.intents) do
		my_input_context:set_intent(k, v)
	end
	
	return my_input_context
end

function create_particle(entries)
	local my_particle = particle()
	rewrite(my_particle, entries, { model = true })
	
	if type(entries.model) ~= "userdata" then
		entries.model = create_sprite( entries.model )
	end
	
	table.insert(polygon_particle_userdatas_saved, entries.model)
	my_particle.model = entries.model
	
	return my_particle
end

function create_emission(entries)
	local my_emission = emission()
	rewrite(my_emission, entries, { particle_templates = true, particle_render_template = true })
	
	for k, v in pairs(entries.particle_templates) do
		my_emission:add_particle_template(create_particle(v))
	end
	
	my_emission.particle_render_template = create(render_component, entries.particle_render_template)
	return my_emission
end

function create_particle_effect(entries)
	local my_effect = particle_effect()
	
	for k, v in pairs(entries) do
		my_effect:add(create_emission(v))
	end
	
	return my_effect
end
	
function create_particle_emitter_info(entries)
	local my_info = particle_emitter_info()
	
	for k, v in pairs(entries.effects_subscribtion) do
		my_info:add(k, create_particle_effect(v))
	end
	
	return my_info
end

function create_scriptable_info(entries)
	local my_info = scriptable_info()
	
	for k, v in pairs(entries.scripted_events) do
		my_info:set(k, v)
	end
	
	return my_info
end

function set_physics_info(my_body_data, entries)
	if entries ~= nil then
		recursive_write(my_body_data, entries, { "vertices" } )
		
		if my_body_data.shape_type == physics_info.POLYGON then
			if type(entries.vertices) == "userdata" then
				my_body_data:from_renderable(entries.vertices)
			elseif type(entries.vertices) == "table" then
				print("VERTICES FROM TABLE NOT IMPLEMENTED YET")
				--for k, v in pairs(entries.vertices) do 
				--	my_body_data:add_vertex(v)
				--end
			end
		end
	end
end

function create_steering(entries)
	local my_behaviour = (entries.behaviour_type)()
	
	recursive_write(my_behaviour, entries, { behaviour_type = true, current_target = true, optional_alignment = true })
	
	
	if entries.current_target ~= nil then 
		my_behaviour.current_target:set(entries.current_target) 
	end
	if entries.optional_alignment ~= nil then 
		my_behaviour.optional_alignment:set(entries.optional_alignment) 
	end
	
	return my_behaviour
end

function create_behaviour_tree(entries)
	--local my_allocator = behaviour_tree_allocator()
	local out_my_nodes = {}
	
	for k, v in pairs(entries.decorators) do 
		out_my_nodes[k] = (v.decorator_type)()
		rewrite(out_my_nodes[k], v, { decorator_type = true })
		
		--if v.base_node ~= nil then
		--	out_my_nodes[k].base_node = out_my_nodes[v.base_node]
		--end
		
		out_my_nodes[k].name = k
	end
	
	for k, v in pairs(entries.decorators) do
		if v.next_decorator then
			out_my_nodes[k].next_decorator = out_my_nodes[v.next_decorator]
		end
	end
	
	for k, v in pairs(entries.nodes) do
		out_my_nodes[k] = behaviour_node()
		rewrite(out_my_nodes[k], v, { decorator_chain = true } )
		
		if v.decorator_chain ~= nil then
			
			out_my_nodes[k].decorator_chain = out_my_nodes[v.decorator_chain]
			--print(out_my_nodes[k].decorator_chain.maximum_running_time_ms)
		end
		
		out_my_nodes[k].name = k
	end
	
	for root, v in pairs(entries.connections) do
		--print("parent is: " .. root)
		for i=1, #v do
			
			--print(i, v[i])
			out_my_nodes[root]:add_child(out_my_nodes[v[i]])
		end
	end
	
	--my_allocator:create_flattened_tree(out_my_nodes[entries.root])
	--
	--for k, v in pairs(out_my_nodes) do
	--	v = my_allocator:retrieve_behaviour(v)
	--end
	
	return out_my_nodes
end

function create_gun(entries)
	--print("creating gun..")
	local temp = create_entity { gun = entries }
	local new_gun = gun_component(temp.gun)
	
	local msg = destroy_message()
	msg.subject = temp
	world:post_message(msg)
	
	return new_gun
end

