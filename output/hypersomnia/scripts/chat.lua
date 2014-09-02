function handle_incoming_chat(client)
	local msgs = client.entity_system_instance.messages["REMOTE_CHAT_MESSAGE"]
	
	for i=1, #msgs do
		msgs[i].data.nickname:add(58)
		msgs[i].data.nickname:add(32)
	
		client.my_gui.recent_messages:append_message( { 
			{ str = msgs[i].data.nickname, color = rgba(0, 255, 255, 255) }, 
			{ str = msgs[i].data.message, color = rgba(255, 255, 255, 255) } 
			
			}
			
			, 
			
			false)
		
		if not client.my_gui.content_chatbox:is_focused() then
			client.my_gui.content_chatbox:set_caret(client.my_gui.content_chatbox:get_length(), false)
			client.my_gui.content_chatbox:view_caret()
		end
	end
end