function handle_incoming_chat(client)
	local msgs = client.entity_system_instance.messages["REMOTE_CHAT_MESSAGE"]
	
	for i=1, #msgs do
		msgs[i].data.nickname:add(58)
		msgs[i].data.nickname:add(32)
		
		local chat_msg = { wstr = msgs[i].data.message, color = rgba(255, 255, 255, 255) }
		local chat_nick = { wstr = msgs[i].data.nickname, color = rgba(0, 255, 255, 255) }
	
		client.my_gui.recent_messages:append_message( format_text ({ chat_nick, chat_msg }, true) )
	
		if not client.my_gui.content_chatbox:is_focused() then
			client.my_gui.content_chatbox:set_caret(client.my_gui.content_chatbox:get_length(), false)
			client.my_gui.content_chatbox:view_caret()
		end
	end
end