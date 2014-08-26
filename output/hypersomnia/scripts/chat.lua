function handle_incoming_chat(client)
	local msgs = client.entity_system_instance.messages["CHAT_MESSAGE"]
	
	for i=1, #msgs do
		local wvec = msgs[i].data.message
		wvec:add(13)
		client.content_chatbox:append_text(wvec, rgba(0, 255, 0, 255))
	end
end