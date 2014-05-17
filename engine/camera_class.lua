camera_class = inherits_from (entity_class)

function camera_class:constructor(parent_entity)
	entity_class.constructor(self, parent_entity)
	self.current_zoom_level = 0
	self.current_zoom_multiplier = 1
end

function camera_class:set_zoom_level(new_zoom_level)
	self.current_zoom_level = new_zoom_level
	
	local mult = 1 + (new_zoom_level / 1000)
	local new_w = config_table.resolution_w*mult
	local new_h = config_table.resolution_h*mult
	self.current_zoom_multiplier = mult
	
	self.parent_entity:get().camera.size = vec2(new_w, new_h)
	self.parent_entity:get().camera.max_look_expand = vec2(new_w, new_h)/2
end

function camera_class:get_zoom_level()
	return self.current_zoom_level
end