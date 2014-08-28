recent_messages_class = inherits_from()

function recent_messages_class:constructor(subject_textbox, second_textbox)
	self.subject_textbox = subject_textbox
	self.second_textbox = second_textbox
	
	self.recent_messages = {}
	
	self.fading_duration = 1000
end

function recent_messages_class:append_message(formatted, raw_string, message_duration)
	local message_len = 0
	
	if not message_duration then
		message_duration = 6000
	end
	
	formatted[#formatted].str:add(13)
	
	for i=1, #formatted do
		if raw_string then
			formatted[i].str = towchar_vec(formatted[i].str)
		end
		
		self.subject_textbox:append_text(formatted[i].str, formatted[i].color)
		
		if self.second_textbox then
			self.second_textbox:append_text(formatted[i].str, formatted[i].color)
		end
	
		message_len = message_len + formatted[i].str:size()
	end
	
	self.recent_messages[#self.recent_messages+1] = {
		length = message_len,
		duration_timer = expiration_timer:create(message_duration)
	}	
end

function recent_messages_class:loop()
	local oldest = self.recent_messages[1]
	
	if oldest and oldest.duration_timer:expired() then
		if not oldest.alpha_timer then
			oldest.alpha_timer = timer()
		end
		
		local target_alpha = 255 - 255*oldest.alpha_timer:get_milliseconds()/self.fading_duration
		
		if target_alpha <= 0 then 
			self.subject_textbox:set_caret(0, false)
			self.subject_textbox:set_caret(oldest.length, true)
			self.subject_textbox:backspace()
			table.remove(self.recent_messages, 1)
		else
			self.subject_textbox:set_alpha_range(0, oldest.length, target_alpha)
		end
	end
end
