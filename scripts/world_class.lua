world_class = inherits_from {}

function world_class:constructor()
	self.world_inst = world_instance()
	-- shortcut 
	self.world = self.world_inst.world
	
	self.substep_callbacks = {}
	
	-- shortcuts for systems
	self.input_system = self.world_inst.input_system
	self.visibility_system = self.world_inst.visibility_system
	self.pathfinding_system = self.world_inst.pathfinding_system
	self.render_system = self.world_inst.render_system
	self.physics_system = self.world_inst.physics_system
	
	self.physics_system.substepping_routine = function(owner)	
		local my_instance = self.world_inst
	
		for i=1, #self.substep_callbacks do
			self.substep_callbacks[i]()
		end
		
		my_instance.steering_system:substep(owner)
		my_instance.movement_system:substep(owner)
		my_instance.damage_system:process_entities(owner)
		my_instance.damage_system:process_events(owner)
		
		self:handle_message_callbacks()
		
		my_instance.destroy_system:consume_events(owner)
		owner:flush_message_queues()
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
		"damage_message",
		"intent_message",
		"collision_message",
		"shot_message"
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

function world_class:process_all_systems()
	local my_instance = self.world_inst
	local world = my_instance.world
	
	world:validate_delayed_messages()
	world:flush_message_queues()
	local steps_made = 0
	
	-- physics must run first because the substep routine may flush message queues
	if not self.is_paused then 
		steps_made = my_instance.physics_system:process_entities(world)
    end
    
	my_instance.input_system:process_entities(world)
	my_instance.camera_system:consume_events(world)
    
	if not self.is_paused then
		my_instance.movement_system:process_entities(world)
	end
     
	my_instance.camera_system:process_entities(world)
    
	
	
	if not self.is_paused then
		my_instance.behaviour_tree_system:process_entities(world)
	end
	my_instance.lookat_system:process_entities(world)
	my_instance.chase_system:process_entities(world)
	my_instance.crosshair_system:process_entities(world)
	
	if not self.is_paused then
		my_instance.gun_system:process_entities(world)
		my_instance.damage_system:process_entities(world)
		my_instance.particle_group_system:process_entities(world)
		my_instance.animation_system:process_entities(world)
		my_instance.visibility_system:process_entities(world)
		my_instance.pathfinding_system:process_entities(world)
		my_instance.damage_system:process_events(world)
	end	
	
	self:handle_message_callbacks()
	
	my_instance.destroy_system:consume_events(world)
	
	if not self.is_paused then
		my_instance.movement_system:consume_events(world)
		my_instance.animation_system:consume_events(world)
	end
	
	my_instance.crosshair_system:consume_events(world)
	
	if not self.is_paused then
		my_instance.gun_system:consume_events(world)
		my_instance.particle_emitter_system:consume_events(world)
	end
	
	my_instance.camera_system:process_rendering(world) 
	
	return steps_made
end

	-- default loop
function world_class:loop()
	return self:process_all_systems()
end