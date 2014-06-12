-- initialize gameplay libraries
dofile "hypersomnia\\scripts\\game\\input.lua"
dofile "hypersomnia\\scripts\\game\\layers.lua"
dofile "hypersomnia\\scripts\\game\\camera.lua"

-- archetypes
dofile "hypersomnia\\scripts\\archetypes\\basic_player.lua"

-- resource handling utilities
dofile "hypersomnia\\scripts\\resources\\animations.lua"


sample_scene = scene_class:create()
sample_scene:load_map("hypersomnia\\data\\maps\\sample_map", "hypersomnia\\scripts\\loaders\\basic_map_loader")


print "Initialization is OK."
-- main loop

while true do
	sample_scene:loop()
	
	if (client:receive(received)) then
		local message_type = received:byte(0)
	
		if message_type == network_message.ID_CONNECTION_REQUEST_ACCEPTED then
			print("Our connection request has been accepted.");
		
			bsOut = BitStream()
			bsOut:WriteByte(UnsignedChar(network_message.ID_GAME_MESSAGE_1))
			WriteCString(bsOut, "Hello world")
			client:send(bsOut, send_priority.HIGH_PRIORITY, send_reliability.RELIABLE_ORDERED, 0, received:guid(), false)
		elseif message_type == network_message.ID_NO_FREE_INCOMING_CONNECTIONS then
			print("The server is full.\n")
		elseif message_type == network_message.ID_DISCONNECTION_NOTIFICATION then
			print("A client has disconnected.\n")
		elseif message_type == network_message.ID_CONNECTION_LOST then
			print("A client lost the connection.\n")
		elseif message_type == network_message.ID_GAME_MESSAGE_1 then
			rs = RakString()
			bsIn = received:get_bitstream()
			bsIn:IgnoreBytes(1)
			bsIn:ReadRakString(rs)
			print("Game message received: " .. rs:C_String())
		else
			print("Message with identifier " .. message_type .. " has arrived.")
		end	
	end
	
	if call_once_after_loop ~= nil then
		call_once_after_loop()
		call_once_after_loop = nil
	end
end

