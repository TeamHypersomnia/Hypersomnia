dofile "config.lua"

ENGINE_DIRECTORY = "..\\..\\Augmentations\\scripts\\"
dofile (ENGINE_DIRECTORY .. "load_libraries.lua")

client = network_interface()
client:connect("127.0.0.1", 37017)

received = network_packet()

network_message.ID_GAME_MESSAGE_1 = network_message.ID_USER_PACKET_ENUM + 1

while true do
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
end

-- enter the game
dofile "hypersomnia\\scripts\\start.lua"