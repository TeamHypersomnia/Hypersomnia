camera_class = inherits_from ()

function camera_class:constructor()
	self.current_zoom_level = 0
	self.current_zoom_multiplier = 1
	
	self.min_zoom = -2000 
	self.max_zoom = 2000

	self.movement_smoother = smooth_value_field()
	self.movement_smoother.averages_per_sec = 5
end

function camera_class:set_zoom_level(new_zoom_level)
	self.current_zoom_level = new_zoom_level
	
	local mult = 1 + (new_zoom_level / 1000)
	local new_w = config_table.resolution_w*mult
	local new_h = config_table.resolution_h*mult
	self.current_zoom_multiplier = mult
	
	self.parent_entity.camera.size = vec2(new_w, new_h)
	self.parent_entity.camera.max_look_expand = vec2(new_w, new_h)/2
end

function camera_class:get_zoom_level()
	return self.current_zoom_level
end

function camera_class:chase_player(target_player, target_crosshair)
	self.parent_entity.chase:set_target(target_player)
	self.parent_entity.camera.player:set(target_player)
	self.parent_entity.camera.crosshair:set(target_crosshair)

	self.chased_player = target_player
end

function camera_class:tick(chase_player)
	if self.chased_player then
		local target_value = to_pixels(self.chased_player.physics.body:GetLinearVelocity())

		if target_value:length() > 20 then
			target_value:set_length(20)
		end

		if target_value:length() < self.movement_smoother.value:length() then
			self.movement_smoother.averages_per_sec = 8
		else
			self.movement_smoother.averages_per_sec = 5
		end 
		
		self.movement_smoother.target_value = target_value * (-1)
		self.movement_smoother:tick()
	
		self.parent_entity.chase.offset = vec2(self.movement_smoother.value.x, self.movement_smoother.value.y)
	end
end

function camera_class:intent_message(message)
	if message.intent == custom_intents.ZOOM_CAMERA then	
		local zoom_level = self:get_zoom_level()
		
		zoom_level = zoom_level-message.wheel_amount
		if zoom_level < self.min_zoom then zoom_level = self.min_zoom end
		if zoom_level > self.max_zoom then zoom_level = self.max_zoom end
		self:set_zoom_level(zoom_level)
	elseif message.intent == intent_message.SWITCH_LOOK then	
		--self.parent_entity.camera.enable_smoothing = not self.parent_entity.camera.enable_smoothing
	end
	
	return false
end