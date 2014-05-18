world_class = inherits_from {}
current_world = nil

function world_class:constructor()
	self.world_inst = world_instance()
	-- shortcut 
	self.world = self.world_inst.world
	
	self.entity_system_instance = entity_system:create(self)
	
	-- shortcuts for systems
	self.input_system = self.world_inst.input_system
	self.visibility_system = self.world_inst.visibility_system
	self.pathfinding_system = self.world_inst.pathfinding_system
	self.render_system = self.world_inst.render_system
	self.physics_system = self.world_inst.physics_system
	
	-- other internals
	self.group_by_entity = {}	
	self.polygon_particle_userdatas_saved = {}	
	
	-- convenience shortcuts for entity system
	self.global_entity_table = self.entity_system_instance.entity_table
	self.global_message_table = self.entity_system_instance.message_table
		
	self.is_paused = false
end

-- shortcut
function world_class:create_entity_table(...)
	return self.entity_system_instance:create_entity_table(...)
end

function world_class:set_current()
	current_world = self
	augmentations_main_loop_callback = function() return self:loop() end
end

function world_class:process_all_systems()
	local my_instance = self.world_inst
	local world = my_instance.world
	
	world:validate_delayed_messages()
	world:flush_message_queues()
	
	if not self.is_paused then 
		my_instance.physics_system:process_entities(world)
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
	
	my_instance.script_system:process_events(world)
	my_instance.script_system:process_entities(world)    
    
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
    
end

	-- default loop
function world_class:loop()
	self:process_all_systems()
	
	return self.world_inst.input_system.quit_flag
end