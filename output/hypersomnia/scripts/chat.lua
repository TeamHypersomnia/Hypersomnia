function handle_incoming_chat(client)
	local msgs = client.entity_system_instance.messages["REMOTE_CHAT_MESSAGE"]
	
	for i=1, #msgs do
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
		
		client.my_gui.recent_messages:append_message_separate( format_text({ chat_nick, chat_msg }, true), format_text (target_text, true) )
	
		if not client.my_gui.content_chatbox:is_focused() then
			client.my_gui.content_chatbox:set_caret(client.my_gui.content_chatbox:get_length(), false)
			client.my_gui.content_chatbox:view_caret()
		end
	end
end