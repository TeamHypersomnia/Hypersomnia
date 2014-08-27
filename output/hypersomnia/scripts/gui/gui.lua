gui_class = inherits_from()

function gui_class:constructor(camera_rect, world_object)
	self.gui_enabled = true
	
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
			intent_message = function(self, message)
				if message.state_flag then
					if message.intent == custom_intents.ENABLE_GUI then
						print "enabling"
					elseif message.intent == custom_intents.ENTER_CHAT then
						print "entering"
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
	
	local blurring_callback = function()
		print "blurring gui.."
	
		set_border(self.content_chatbox, "released", 0, rgba(255, 255, 255, 150))
		set_color(self.content_chatbox, "released", rgba(0, 0, 0, 0))
		set_color(self.main_chatbox, "released", rgba(0, 0, 0, 30))
	end
	-- blurring gui
	self.focusable_bg:set_focus_callback(blurring_callback)	
	
	-- focusing gui
	self.focusable_bg:set_blur_callback(function()
		print "focusing gui.."
	
		set_border(self.content_chatbox, "released", 1, rgba(255, 255, 255, 30))
		set_color(self.content_chatbox, "released", rgba(0, 0, 0, 50))
		set_color(self.main_chatbox, "released", rgba(0, 0, 0, 100))
	end)
	
	self.focusable_bg:focus()
	--set_color(self.main_chatbox, "focused", rgba(0, 255, 255, 100))
	--
	--set_border(self.content_chatbox, "released", 0, rgba(0, 255, 255, 0))
	
	self.main_chatbox:set_command_callback(function(wvec)
		if wvec:size() > 0 then
			if self.server_guid then
				self.server:send(protocol.make_reliable_bs(protocol.write_msg("CHAT_MESSAGE", {
					message = wvec
				})), send_priority.LOW_PRIORITY, send_reliability.RELIABLE_ORDERED, 0, self.server_guid, false)
			end
		end
		
	end)
end