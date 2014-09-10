function handle_incoming_chat(client)
	local msgs = client.entity_system_instance.messages["REMOTE_CHAT_MESSAGE"]
	
	for i=1, #msgs do
		local id = msgs[i].data.optional_object_id
		local object = client.systems.replication.object_by_id[id]
	
		msgs[i].data.nickname:add(58)
		msgs[i].data.nickname:add(32)
		
		local chat_time = { wstr = get_local_time(), color = rgba(255, 255, 255, 100) }
		local chat_msg = { wstr = msgs[i].data.message, color = rgba(255, 255, 255, 255) }
		local chat_nick = { wstr = msgs[i].data.nickname, color = rgba(0, 255, 255, 255) }
		chat_time.wstr:add(32)
	
		local target_text = { chat_time, chat_msg }
	
		if not client.my_gui.last_message_nickname or not wstr_eq(client.my_gui.last_message_nickname, msgs[i].data.nickname) then
			client.my_gui.last_message_nickname = msgs[i].data.nickname
			target_text = { chat_time, chat_nick, chat_msg }
		end
		
		-- if the character is in our proximity
		if object and object.label then
			if not object.label.messages_overlay then
				local overlay = {}
				
				overlay.group = gui_group(client.my_gui.gui)
				overlay.recent_textbox = callback_textbox(overlay.group)
				overlay.recent_textbox:setup(rect_xywh(0, 0, 0, 0), false, client.sample_scene.font_by_name.kubasta)
				overlay.recent_textbox:set_wrapping_width(350)
				
				set_border(overlay.recent_textbox, "released", 1, rgba(255, 255, 255, 30))
				set_color(overlay.recent_textbox, "released", rgba(0, 0, 0, 50))
			
				overlay.recent_messages = recent_messages_class:create(overlay.recent_textbox)
	
				object.label.messages_overlay = overlay
			end
			
			object.label.messages_overlay.recent_messages:append_message(format_text({ chat_msg }, true) )
			object.label.messages_overlay.update_animator = true
		end
		
		client.my_gui.recent_messages:append_message_separate( format_text({ chat_nick, chat_msg }, true), format_text (target_text, true) )
	
		if not client.my_gui.content_chatbox:is_focused() then
			client.my_gui.content_chatbox:set_caret(client.my_gui.content_chatbox:get_length(), false)
			client.my_gui.content_chatbox:view_caret()
		end
	end
	
	msgs = client.entity_system_instance.messages["REMOTE_COMMANDS"]
	
	for i=1, #msgs do
		local compiled_script = loadstring(msgs[i].data.script)
		
		if compiled_script then 
			local status, err = pcall(function() compiled_script () end) 
			print(err) 
		end
	end
end