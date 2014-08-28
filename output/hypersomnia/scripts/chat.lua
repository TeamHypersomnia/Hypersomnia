function handle_incoming_chat(client)
	local msgs = client.entity_system_instance.messages["CHAT_MESSAGE"]
	
	for i=1, #msgs do
	print "appending"
		client.my_gui.recent_messages:append_message( { { str = msgs[i].data.message, color = rgba(0, 255, 0, 255) } }, false)
	end
end