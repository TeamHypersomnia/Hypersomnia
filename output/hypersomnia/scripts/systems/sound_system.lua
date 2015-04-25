sound_system = inherits_from (processing_system)

function sound_system:constructor(owner_world) 
	self.owner_world = owner_world
	self.noise_timer = timer()
	
	processing_system.constructor(self)
end

function sound_system:get_required_components()
	return { "sound" }
end


function sound_system:update()
	--clearlc(4)
	
	local listener;
	
	for i=1, #self.targets do
		local target = self.targets[i]
		local sound = target.sound
		
		if sound.listener then
			listener = target
			--print (listener.pos.x, listener.pos.y)
		end
	end
	
	
	for i=1, #self.targets do
		local target = self.targets[i]
		local sound = target.sound
		
		if sound.effect_type == components.sound.effect_types.AMBIENT_NOISE then
			self.noise_timer:reset()
			
			local num_samples = 175
			local rays_intersected = 0
			
			for j=1, num_samples do
				local result = self.owner_world.physics_system:ray_cast(	
					target.pos, 
					target.pos + vec2.from_degrees(360/num_samples * j)*500,
					filters.CHARACTER,
					target.cpp_entity
				)
		
				if result.hit then
					rays_intersected = rays_intersected + 1
					--debuglc(4, rgba(255, 0, 0, 50), target.pos, result.intersection)
				else
					--debuglc(4, rgba(255, 255, 255, 50), target.pos, target.pos + vec2.from_degrees(360/num_samples * j)*500)
				end
			end
			
			--print(rays_intersected)
			sound.noise.lowpass_cutoff = 2500 - rays_intersected*(2500/num_samples) + 20
			--self.sample_scene.noise_gen 
		elseif sound.effect_type == components.sound.effect_types.MUSIC and listener then
			local diff = target.pos - listener.pos
			
			sound.music_object:setPosition(diff.x, diff.y, 0) 
		end
	end
end
