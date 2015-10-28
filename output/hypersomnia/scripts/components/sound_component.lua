components.sound = inherits_from()

components.sound.effect_types = create_enum {
	"SOUND",
	"MUSIC",
	"AMBIENT_NOISE"
}

function components.sound:constructor(init_table) 
	recursive_write(self, init_table)
	
--	if self.effect_type == components.sound.effect_types.AMBIENT_NOISE then
--		self.noise = noise_generator()
--		self.noise.lowpass_cutoff = 50
--		self.noise:play()
--	elseif self.effect_type == components.sound.effect_types.MUSIC then
--		self.music_object:setRelativeToListener(true)
--		self.music_object:setMinDistance(700)
--		self.music_object:setAttenuation(1)
--		
--		--self.music_object:setPosition(1, 0, 0) 
--		self.music_object:play()
--	end
end