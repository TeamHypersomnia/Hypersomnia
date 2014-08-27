gui_class = inherits_from()

function cprint(str, color, target_box)
	local text_color = rgba(255, 255, 0, 255)
	if color then text_color = color end
	local vec_str = towchar_vec(str)
	vec_str:add(13)
	
	if target_box == nil then
		target_box = client_scenes[CURRENT_CLIENT_NUMBER].my_gui.content_chatbox
	end
	
	target_box:append_text(vec_str, text_color)
end

function gui_class:set_enabled(flag)
	if self.gui_enabled ~= flag then
		self.gui_enabled = flag
		
		if flag then
			if self.press_info_written then
				self.main_chatbox:clear_text()
				self.press_info_written = false
			end
			
			main_input_context.enabled = false
			gui_input_context.enabled = true
		
			set_border(self.content_chatbox, "released", 1, rgba(255, 255, 255, 30))
			set_color(self.content_chatbox, "released", rgba(0, 0, 0, 50))
			set_color(self.main_chatbox, "released", rgba(0, 0, 0, 100))
		else
			main_input_context.enabled = true
			gui_input_context.enabled = true
			
			set_border(self.content_chatbox, "released", 0, rgba(255, 255, 255, 150))
			set_color(self.content_chatbox, "released", rgba(0, 0, 0, 0))
			set_color(self.main_chatbox, "released", rgba(0, 0, 0, 30))
			
			self.gui:blur()
			
			if self.main_chatbox:is_clean() then
				cprint("Press Enter to chat or Alt to enable GUI...", rgba(255, 255, 255, 120), self.main_chatbox)
				self.press_info_written = true
			end
		end
		
		
		local val = 0
		if flag then val = 1 end
		set_cursor_visible(val)
	end
end

function gui_class:constructor(camera_rect, world_object, owner_client)
	self.owner_client = owner_client
	self.gui_enabled = true
	
	self.press_info_written = false
	
	world_object.input_system.event_callback = function () 
		if self.gui_enabled then
			self.gui:poll_events()
		end
	end
	
	self.intent_handler = world_object:create_entity {
		input = {
			custom_intents.ENTER_CHAT,
			custom_intents.ENABLE_GUI
		},
		
		script = {
			intent_message = function(this, message)
				if message.state_flag then
					if message.intent == custom_intents.ENABLE_GUI then
						self:set_enabled(not self.gui_enabled)
					elseif message.intent == custom_intents.ENTER_CHAT then
						if not self.ignore_intent and not self.main_chatbox:is_focused() then
							--self:set_enabled(false)
						--else						
							self:set_enabled(true)
							self.main_chatbox:focus()	
						end
						
						self.ignore_intent = nil
						--end
					end
				end
			end
		}
	}

	self.gui = hypersomnia_gui(global_gl_window)
	self.gui:setup(vec2(camera_rect.w, camera_rect.h))
	
	self.focusable_bg = callback_rect(self.gui)
	self.focusable_bg:setup(rect_xywh(0, 0, camera_rect.w, camera_rect.h), true)
	
	self.content_chatbox = callback_textbox(self.gui)
	self.content_chatbox:setup(rect_xywh(20, camera_rect.h - 500 + 100 + 150, 350, 150), false)
	
	self.main_chatbox = callback_textbox(self.gui)
	self.main_chatbox:setup(rect_xywh(20, camera_rect.h - 160 + 90, 350, 35), true)
	
	--set_color(self.content_chatbox, "released", rgba(0, 0, 0, 50))
	--set_color(self.main_chatbox, "released", rgba(0, 0, 0, 100))
	--set_color(self.focusable_bg, "released", rgba(0, 0, 0, 0))
	
	self.focusable_bg:set_focus_callback(function() self:set_enabled(false) end)
	
	-- focusing gui like this should never happen; it is only possible through enter/alt
	--self.focusable_bg:set_blur_callback( function() self:set_enabled(true) end)	
	
	self:set_enabled(false)
	--set_color(self.main_chatbox, "focused", rgba(0, 255, 255, 100))
	--
	--set_border(self.content_chatbox, "released", 0, rgba(0, 255, 255, 0))
	
	self.main_chatbox:set_command_callback(function(wvec)
		self.ignore_intent = true
		
		if wvec:size() > 0 then
			if self.owner_client.server_guid then
				self.owner_client.server:send(protocol.make_reliable_bs(protocol.write_msg("CHAT_MESSAGE", {
					message = wvec
				})), send_priority.LOW_PRIORITY, send_reliability.RELIABLE_ORDERED, 0, self.owner_client.server_guid, false)
			end
			
			self.main_chatbox:clear_text()
		else
			self:set_enabled(false)
		end
		
	end)
end