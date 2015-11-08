simulation_world_class = inherits_from (world_class)

function simulation_world_class:constructor()
	self.world_inst = world_instance()
	-- shortcut 
	self.world = self.world_inst.world
	
	self.prestep_callbacks = {}
	self.poststep_callbacks = {}
	
	-- shortcuts for systems
	self.physics_system = self.world_inst.physics_system
	self.physics_system:enable_listener(false)
	self.physics_system.enable_interpolation = 0
	
	self.physics_system.prestepping_routine = function(owner)	
		local my_instance = self.world_inst
	
		for i=1, #self.prestep_callbacks do
			self.prestep_callbacks[i]()
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
end

function simulation_world_class:process_steps(steps)
	local my_instance = self.world_inst
	local world = my_instance.world

	self:clear_all_queues()
	
	my_instance.physics_system:process_steps(world, steps)
end