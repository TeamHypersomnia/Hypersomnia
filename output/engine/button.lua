button_class = inherits_from {}

function button_class:constructor()
end

function button_class:set(pos, size, callbacks)
	self.pos = pos
	self.size = size
	
	self.callbacks = callbacks
	self.was_hovered = false
end

--function button_class:set_from_xywh(rect, callbacks)
--	self.size = vec2(rect.w, rect.h)
--	self.pos = vec2(rect.x + rect.w / 2, rect.y + rect.h / 2)
--	
--	self.callbacks = callbacks
--	self.was_hovered = false
--end

function button_class:construct_xywh()
	return rect_xywh(self.pos.x, self.pos.y, self.size.x, self.size.y)
end

function button_class:check_mouse_events(message, crosshair_pos, mousemove_intent, mouseclick_intent)
	local is_hovering = self:construct_xywh():hover(crosshair_pos)
	
	if message.intent == mousemove_intent then
		if is_hovering and not self.was_hovered then
			if self.callbacks.mousein ~= nil then self.callbacks.mousein() end
			self.was_hovered = true
		end
		
		if not is_hovering and self.was_hovered then
			if self.callbacks.mouseout ~= nil then self.callbacks.mouseout() end
			self.was_hovered = false
		end
	end
	
	if message.intent == mouseclick_intent and message.state_flag then
		if is_hovering then
			if self.callbacks.mouseclick ~= nil then 
				play_sound(button_clicked_snd)
				self.callbacks.mouseclick() 
			end
		end
	end
end