camera_class = inherits_from ()

function camera_class:constructor()
	self.current_zoom_level = 0
	self.current_zoom_multiplier = 1
	
	self.min_zoom = -2000 
	self.max_zoom = 2000
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

function camera_class:intent_message(message)
	if message.intent == custom_intents.ZOOM_CAMERA then	
		local zoom_level = self:get_zoom_level()
		
		zoom_level = zoom_level-message.wheel_amount
		if zoom_level < self.min_zoom then zoom_level = self.min_zoom end
		if zoom_level > self.max_zoom then zoom_level = self.max_zoom end
		self:set_zoom_level(zoom_level)
	end
	
	return false
end