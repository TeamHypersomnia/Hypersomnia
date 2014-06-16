client_screen = inherits_from ()

network_message.ID_MOVEMENT = network_message.ID_USER_PACKET_ENUM + 1
network_message.ID_NEW_PLAYER = network_message.ID_USER_PACKET_ENUM + 2

function client_screen:constructor(camera_rect)
	self.sample_scene = scene_class:create()
	self.sample_scene:load_map("hypersomnia\\data\\maps\\sample_map.lua", "hypersomnia\\scripts\\loaders\\basic_map_loader.lua")
	
	self.sample_scene.world_camera.camera.screen_rect = camera_rect
	
	self.client = network_interface()
	self.client:connect("127.0.0.1", 37017)
	
	self.received = network_packet()
end

function client_screen:loop()
	self.sample_scene:loop()
	
	-- handle networking
	
	if (self.client:receive(self.received)) then
		local message_type = self.received:byte(0)
	
		if message_type == network_message.ID_CONNECTION_REQUEST_ACCEPTED then
			print("Our connection request has been accepted.");
		elseif message_type == network_message.ID_NO_FREE_INCOMING_CONNECTIONS then
			print("The server is full.\n")
		elseif message_type == network_message.ID_DISCONNECTION_NOTIFICATION then
			print("A client has disconnected.\n")
		elseif message_type == network_message.ID_CONNECTION_LOST then
			print("A client lost the connection.\n")
			
		elseif message_type == network_message.ID_NEW_PLAYER then
			local new_guid = RakNetGUID()
			local bsIn = self.received:get_bitstream()
			bsIn:IgnoreBytes(1)
			ReadRakNetGUID(bsIn, new_guid)
			print("Game message received: " .. message_type)
			
		else
			print("Message with identifier " .. message_type .. " has arrived.")
		end	
	end
end