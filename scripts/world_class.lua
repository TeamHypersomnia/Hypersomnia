world_class = inherits_from {}

function world_class:constructor()
	self.world_inst = world_instance()
	-- shortcut 
	self.world = self.world_inst.world
	
	self.prestep_callbacks = {}
	self.poststep_callbacks = {}
	
	-- shortcuts for systems
	self.input_system = self.world_inst.input_system
	self.visibility_system = self.world_inst.visibility_system
	self.pathfinding_system = self.world_inst.pathfinding_system
	self.render_system = self.world_inst.render_system
	self.physics_system = self.world_inst.physics_system
	
	self.physics_system.prestepping_routine = function(owner)
		local my_instance = self.world_inst
	
		local dt = self.physics_system:get_timestep_ms()
		for i=1, #self.prestep_callbacks do
			self.prestep_callbacks[i](dt)
		end
		
		my_instance.steering_system:substep(owner)
		my_instance.movement_system:substep(owner)
	end
	
	self.physics_system.poststepping_routine = function(owner)
		local my_instance = self.world_inst
		
		local dt = self.physics_system:get_timestep_ms()
		for i=1, #self.poststep_callbacks do
			self.poststep_callbacks[i](dt)
		end
		
		my_instance.destroy_system:consume_events(owner)
	end
	
	
	self.polygon_particle_userdatas_saved = {}	
		
	self.is_paused = false
end

function world_class:get_messages(message_name)
	return _G["get_" .. message_name .. "_queue"](self.world)
end

function world_class:get_messages_filter_components(message_name, needed_components)
	local output_messages = {}
	
	local message_vector = self:get_messages(message_name)
	
	if message_name == "collision_message" then
		if message_vector:size() > 0 then print (message_vector:size()) end
	end
	
	if message_vector:size() > 0 then
		for i=0, message_vector:size()-1 do
			local msg = message_vector:at(i)
			
			local entity_self = msg.subject.script
			
	if message_name == "collision_message" then
		print "matches"
	end
			if entity_self ~= nil then
				local matches = true
				
				for n=1, #needed_components do
					if entity_self[needed_components[n]] == nil then
						matches = false
						break
					end
				end
				
				if matches then
					table.insert(output_messages, msg)
				end
			end
		end
	end
	
	return output_messages			
end

function world_class:handle_message_callbacks()
	local message_names = {
		"intent_message",
		"collision_message"
	}
	
	for i=1, #message_names do
		local message_vector = self:get_messages(message_names[i])
		
		if message_vector:size() > 0 then
			for j=0, message_vector:size()-1 do
				local msg = message_vector:at(j)
				
				local entity_self = msg.subject.script
				
				if entity_self ~= nil then
					local message_callback = entity_self[message_names[i]]
					
					-- call an entity-specific callback for this message
					if message_callback  ~= nil then
						message_callback (entity_self, msg)
					end

					-- call a generic callback for this message
					if entity_self["message"] ~= nil then
						entity_self:message(message_names[i], msg)
					end
				end
			end
		end
	end
	
end

function world_class:clear_queue(message_name)
	self:get_messages(message_name):clear()
end

function world_class:flush_messages()
	self.world_inst.world:validate_delayed_messages()
	self.world_inst.world:flush_message_queues()
end

function world_class:handle_input()
	local my_instance = self.world_inst
	local world = my_instance.world
	
	my_instance.input_system:process_entities(world)
end

function world_class:handle_physics()
	local my_instance = self.world_inst
	local world = my_instance.world
	
	local steps_made = 0
	
	-- physics must run first because the substep routine may flush message queues
	if not self.is_paused then 
		steps_made = my_instance.physics_system:process_entities(world)
    end
	
	return steps_made
end

function world_class:process_all_systems()
	local my_instance = self.world_inst
	local world = my_instance.world
	
	if not self.is_paused then
		my_instance.movement_system:process_entities(world)
	end
     
	if not self.is_paused then
		my_instance.behaviour_tree_system:process_entities(world)
	end
	
	my_instance.lookat_system:process_entities(world)
	my_instance.chase_system:process_entities(world)
	my_instance.crosshair_system:process_entities(world)
	
	if not self.is_paused then
		my_instance.particle_group_system:process_entities(world)
		my_instance.animation_system:process_entities(world)
		my_instance.visibility_system:process_entities(world)
		my_instance.pathfinding_system:process_entities(world)
	end	
	
	
	return steps_made
end

function world_class:consume_events()
	local my_instance = self.world_inst
	local world = my_instance.world
	
	my_instance.camera_system:consume_events(world)
	
	self:handle_message_callbacks()
	
	my_instance.destroy_system:consume_events(world)
	
	if not self.is_paused then
		my_instance.movement_system:consume_events(world)
		my_instance.animation_system:consume_events(world)
	end
	
	my_instance.crosshair_system:consume_events(world)
	
	if not self.is_paused then
		my_instance.particle_emitter_system:consume_events(world)
	end
	
	self:flush_messages()
end

function world_class:render()
	local my_instance = self.world_inst
	local world = my_instance.world
	
	my_instance.camera_system:process_entities(world)
	my_instance.camera_system:process_rendering(world) 
end