client_screen = inherits_from ()

network_message.ID_INITIAL_STATE = network_message.ID_USER_PACKET_ENUM + 1
network_message.ID_MOVEMENT = network_message.ID_USER_PACKET_ENUM + 2
network_message.ID_NEW_PLAYER = network_message.ID_USER_PACKET_ENUM + 3
network_message.ID_PLAYER_DISCONNECTED = network_message.ID_USER_PACKET_ENUM + 4

function client_screen:constructor(camera_rect)
	self.sample_scene = scene_class:create()
	self.sample_scene:load_map("hypersomnia\\data\\maps\\sample_map.lua", "hypersomnia\\scripts\\loaders\\basic_map_loader.lua")
	
	self.sample_scene.world_camera.camera.screen_rect = camera_rect
	
	self.client = network_interface()
	self.client:connect("127.0.0.1", 37017)
	
	self.received = network_packet()
	
	self.remote_client_map = guid_to_object_map()
end

function client_screen:loop()
	self.sample_scene:loop()
	
	-- handle networking
	
	if (self.client:receive(self.received)) then
		local message_type = self.received:byte(0)
	
		if message_type == network_message.ID_CONNECTION_REQUEST_ACCEPTED then
			self.server_guid = self.received:guid()
			print("Our connection request has been accepted.");
		elseif message_type == network_message.ID_NO_FREE_INCOMING_CONNECTIONS then
			print("The server is full.\n")
		elseif message_type == network_message.ID_DISCONNECTION_NOTIFICATION then
			print("A client has disconnected.\n")
		elseif message_type == network_message.ID_CONNECTION_LOST then
			print("A client lost the connection.\n")
			
			
		elseif message_type == network_message.ID_INITIAL_STATE then
			local bsIn = self.received:get_bitstream()
			bsIn:IgnoreBytes(1)
			local num_players = ReadUint(bsIn)
			
			for i=1, num_players do
				create_remote_player(self.sample_scene, self.sample_scene.teleport_position, ReadRakNetGUID(bsIn))
			end
			
			print "Initial state transferred."
		elseif message_type == network_message.ID_NEW_PLAYER then
		
			local bsIn = self.received:get_bitstream()
			bsIn:IgnoreBytes(1)
			local remote_guid = ReadRakNetGUID(bsIn)
			
			self.remote_client_map:add(remote_guid, create_remote_player(self.sample_scene, self.sample_scene.teleport_position, remote_guid))
			print("New client connected.")
			
		elseif message_type == network_message.ID_PLAYER_DISCONNECTED then
			local bsIn = self.received:get_bitstream()
			bsIn:IgnoreBytes(1)
			local disconnected_guid = ReadRakNetGUID(bsIn)
			
			local remote_self = self.remote_client_map:get(disconnected_guid)
			self.sample_scene.world_object.world:delete_entity(remote_self.parent_entity:get(), nil)
			self.remote_client_map:remove(disconnected_guid)
			
			print("Player disconnected.")
			
		else
			print(network_message.ID_NEW_PLAYER)
			print("Message with identifier " .. message_type .. " has arrived.")
		end	
	end
end

function client_screen:close_connection()
	if self.server_guid ~= nil then
		self.client:close_connection(self.server_guid, send_priority.IMMEDIATE_PRIORITY)
	end
	self.server_guid = nil
end

