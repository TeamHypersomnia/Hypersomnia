world_class = inherits_from {}

function world_class:constructor()
	self.world = WORLD
	
	self.prestep_callbacks = {}
	self.poststep_callbacks = {}
	
	-- shortcuts for systems
	self.input_system = self.world.input_system
	self.visibility_system = self.world.visibility_system
	self.pathfinding_system = self.world.pathfinding_system
	self.render_system = self.world.render_system
	self.physics_system = self.world.physics_system
	
	self.physics_system.prestepping_routine = function(owner)
		local world = self.world
	
		local dt = self.physics_system:get_timestep_ms()
		for i=1, #self.prestep_callbacks do
			self.prestep_callbacks[i](dt)
		end
		
		world.steering_system:substep(owner)
		world.movement_system:substep(owner)
	end
	
	self.physics_system.poststepping_routine = function(owner)
		local world = self.world
		
		local dt = self.physics_system:get_timestep_ms()
		for i=1, #self.poststep_callbacks do
			self.poststep_callbacks[i](dt)
		end
		
		world.destroy_system:consume_events(owner)
	end
	
	self.is_paused = false
end

function world_class:get_messages(message_name)
	return _G["get_" .. message_name .. "_queue"](self.world)
end

function world_class:get_messages_filter_components(message_name, needed_components)
	local output_messages = {}
	
	local message_vector = self:get_messages(message_name)
	
	if message_vector:size() > 0 then
		for i=0, message_vector:size()-1 do
			local msg = message_vector:at(i)
			
			local entity_self = msg.subject.script
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

function world_class:clear_all_queues()
	self:clear_queue("intent_message")
	self:clear_queue("particle_burst_message")
	self:clear_queue("animate_message")
	self:clear_queue("collision_message")
	self:clear_queue("damage_message")
	self:clear_queue("shot_message")
	self:clear_queue("destroy_message")
end

function world_class:handle_input()
	self.world.input_system:process_entities(self.world)
end

function world_class:handle_physics()
	local world = self.world
	
	local steps_made = 0
	
	-- physics must run first because the substep routine may flush message queues
	if not self.is_paused then 
		steps_made = world.physics_system:process_entities(world)
    end
	
	return steps_made
end

function world_class:process_all_systems(is_view)
	local world = self.world

	if is_view == nil then
		is_view = true
	end

	if not self.is_paused then
		if is_view then world.movement_system:process_entities(world) end
	end
     
	if not self.is_paused then
		world.behaviour_tree_system:process_entities(world)
	end
	
	if is_view then world.lookat_system:process_entities(world) end
	world.chase_system:process_entities(world)
	if is_view then world.crosshair_system:process_entities(world) end
	
	if not self.is_paused then
		if is_view then 
			world.particle_group_system:process_entities(world)
			world.animation_system:process_entities(world)
		end

		world.visibility_system:process_entities(world)
		world.pathfinding_system:process_entities(world)
	end	
	
	
	return steps_made
end

function world_class:call_deletes()
	if self:get_messages("destroy_message"):size() > 0 then
	end

	self.world.destroy_system:consume_events(self.world)
end

function world_class:consume_events(is_view)
	local world = self.world

	if is_view == nil then
		is_view = true
	end

	if is_view then
		world.camera_system:consume_events(world)
	end

	self:handle_message_callbacks()
	
	if is_view then
		if not self.is_paused then
			world.movement_system:consume_events(world)
			world.animation_system:consume_events(world)
		end
		
		world.crosshair_system:consume_events(world)
		
		if not self.is_paused then
			world.particle_emitter_system:consume_events(world)
		end
	end
	
	self:clear_queue("intent_message")
	self:clear_queue("particle_burst_message")
	self:clear_queue("animate_message")
end

function world_class:render()
	self.world.camera_system:process_entities(self.world)
	self.world.camera_system:process_rendering(self.world) 
end