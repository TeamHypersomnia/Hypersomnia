function create(module, entries)
	local new_instance = module()
	for k, v in pairs(entries) do
		new_instance[k] = entries[k]
	end
	
	return new_instance
end

function create_options(entries, output)
	output = output or _G
	for k, v in pairs(entries) do
		output[v] = bitflag(k-1)
	end
	
	return entries
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

function create_animation(entries) 
	local my_animation = animation()
	rewrite(my_animation, entries)
	
	for i = 1, #(entries.frames) do
		-- shortcut
		local m = entries.frames[i].model
		if m == nil then
			my_animation:add_frame(create_sprite { image = nil }, entries.frames[i].duration_ms)
		elseif type(m) == "userdata" then
			my_animation:add_frame(create_sprite{ image = m } , entries.frames[i].duration_ms)
		else
			my_animation:add_frame(create_sprite(m), entries.frames[i].duration_ms)
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
	
	for k,v in pairs(entries.intents) do
		my_input_context:set_intent(k, v)
	end
	
	return my_input_context
end

function create_particle(entries)
	local my_particle = particle()
	rewrite(my_particle, entries, { model = true })
	
	my_particle.model = create_sprite( entries.model ) 
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

function set_physics_info(my_body_data, entries)
	if entries ~= nil then
		recursive_write(my_body_data, entries, { "vertices" } )
		
		if my_body_data.type == physics_info.POLYGON and entries.vertices ~= nil then
			for k, v in pairs(entries.vertices) do 
				my_body_data:add_vertex(v)
			end
		end
	end
end
