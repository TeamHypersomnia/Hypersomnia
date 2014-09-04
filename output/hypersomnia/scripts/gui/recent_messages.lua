recent_messages_class = inherits_from()

function recent_messages_class:constructor(subject_textbox, second_textbox)
	self.subject_textbox = subject_textbox
	self.second_textbox = second_textbox
	
	self.recent_messages = {}
	
	self.fading_duration = 1000
end

function recent_messages_class:append_message(formatted, message_duration)
	self:append_message_separate(formatted, formatted, message_duration)
end

function recent_messages_class:append_message_separate(formatted, formatted_second, message_duration)
	local message_len = 0
	
	if not message_duration then
		message_duration = 20000
	end
	
	self.subject_textbox:append_text(formatted, true)
	
	if self.second_textbox then
		self.second_textbox:append_text(formatted_second, true)
	end
		
	self.recent_messages[#self.recent_messages+1] = {
		length = formatted:size(),
		duration_timer = expiration_timer:create(message_duration)
	}	
end

function recent_messages_class:loop()
	local oldest = self.recent_messages[1]
	local was_removed = false
	
	if oldest and oldest.duration_timer:expired() then
		if not oldest.alpha_timer then
			oldest.alpha_timer = timer()
		end
		
		local target_alpha = 255 - 255*oldest.alpha_timer:get_milliseconds()/self.fading_duration
		
		if target_alpha <= 0 then 
			self.subject_textbox:set_caret(0, false)
			self.subject_textbox:set_caret(oldest.length, true)
			self.subject_textbox:backspace()
			was_removed = true
			table.remove(self.recent_messages, 1)
		else
			self.subject_textbox:set_alpha_range(0, oldest.length, target_alpha)
		end
	end
	
	return oldest ~= nil, was_removed
end
