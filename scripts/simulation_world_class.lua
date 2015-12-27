simulation_world_class = inherits_from (world_class)

function simulation_world_class:constructor()
	self.world = hypersomnia_world()
	
	self.prestep_callbacks = {}
	self.poststep_callbacks = {}
	
	-- shortcuts for systems
	self.physics_system = self.world.physics_system
	self.physics_system:enable_listener(false)
	self.physics_system.enable_interpolation = 0
	
	self.physics_system.prestepping_routine = function(owner)	
		for i=1, #self.prestep_callbacks do
			self.prestep_callbacks[i]()
		end
		
		self.world.steering_system:substep(owner)
		self.world.movement_system:substep(owner)
	end
	
	self.physics_system.poststepping_routine = function(owner)
		local dt = self.physics_system:get_timestep_ms()
		for i=1, #self.poststep_callbacks do
			self.poststep_callbacks[i](dt)
		end
		
		self.world.destroy_system:consume_events(owner)
	end
end

function simulation_world_class:process_steps(steps)
	self:clear_all_queues()
	self.world.physics_system:process_steps(world, steps)
end