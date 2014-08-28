function handle_incoming_chat(client)
	local msgs = client.entity_system_instance.messages["CHAT_MESSAGE"]
	
	for i=1, #msgs do
		client.my_gui.recent_messages:append_message( { { str = msgs[i].data.message, color = rgba(0, 255, 0, 255) } }, false)
		
		if not client.my_gui.content_chatbox:is_focused() then
			client.my_gui.content_chatbox:set_caret(client.my_gui.content_chatbox:get_length(), false)
			client.my_gui.content_chatbox:view_caret()
		end
	end
end