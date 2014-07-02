simulation_world_class = inherits_from (world_class)

function simulation_world_class:constructor()
	self.world_inst = world_instance()
	-- shortcut 
	self.world = self.world_inst.world
	
	self.substep_callbacks = {}
	
	-- shortcuts for systems
	self.physics_system = self.world_inst.physics_system
	self.physics_system:enable_listener(false)
	self.physics_system.enable_interpolation = 0
	
	self.physics_system.substepping_routine = function(owner)	
		local my_instance = self.world_inst
	
		for i=1, #self.substep_callbacks do
			self.substep_callbacks[i]()
		end
		
		my_instance.movement_system:substep(owner)
		
		owner:flush_message_queues()
	end
end

function simulation_world_class:process_steps(steps)
	local my_instance = self.world_inst
	local world = my_instance.world
	
	world:validate_delayed_messages()
	world:flush_message_queues()
	
	my_instance.physics_system:process_steps(world, steps)
end