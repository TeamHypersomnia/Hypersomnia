dofile "hypersomnia\\scripts\\network_commands.lua"

client_screen = inherits_from ()

function client_screen:constructor(camera_rect)
	self.sample_scene = scene_class:create()
	self.sample_scene:load_map("hypersomnia\\data\\maps\\sample_map.lua", "hypersomnia\\scripts\\loaders\\basic_map_loader.lua")
	
	self.sample_scene.world_camera.camera.screen_rect = camera_rect
	
	self.client = network_interface()
	self.client:connect(config_table.server_address, config_table.server_port)
	
	self.received = network_packet()
	
	self.remote_client_map = guid_to_object_map()
	
	local this_client = self
	
	self.network_input_listener = self.sample_scene.world_object:create_entity {
		input = {
			intent_message.MOVE_FORWARD,
			intent_message.MOVE_BACKWARD,
			intent_message.MOVE_LEFT,
			intent_message.MOVE_RIGHT
		},
		
		script = {
			intent_message = function(self, message)
				if this_client.server_guid ~= nil then
					local desired_command;
					
					if message.state_flag then 
						desired_command = "+"
					else 
						desired_command = "-" 
					end
					
					local bsOut = BitStream()
					WriteByte(bsOut, protocol.message.COMMAND)
					WriteByte(bsOut, name_to_command[desired_command .. intent_to_name[message.intent]])
					
					this_client.client:send(bsOut, send_priority.IMMEDIATE_PRIORITY, send_reliability.RELIABLE_ORDERED, 0, this_client.server_guid, false)
				end
			end
		
		}
	}
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
			
			
		elseif message_type == protocol.message.INITIAL_STATE then
			local bsIn = self.received:get_bitstream()
			bsIn:IgnoreBytes(1)
			local num_players = ReadUint(bsIn)
			
			
			for i=1, num_players do
				local remote_guid = ReadRakNetGUID(bsIn)
				self.remote_client_map:add(remote_guid, create_remote_player(self.sample_scene, self.sample_scene.teleport_position, remote_guid))
			end
			
			print "Initial state transferred."
		elseif message_type == protocol.message.NEW_PLAYER then
		
			local bsIn = self.received:get_bitstream()
			bsIn:IgnoreBytes(1)
			
			local remote_guid = ReadRakNetGUID(bsIn)
			self.remote_client_map:add(remote_guid, create_remote_player(self.sample_scene, self.sample_scene.teleport_position, remote_guid))
			print("New client connected.")
			
		elseif message_type == protocol.message.PLAYER_DISCONNECTED then
			local bsIn = self.received:get_bitstream()
			bsIn:IgnoreBytes(1)
			local disconnected_guid = ReadRakNetGUID(bsIn)
			
			local remote_self = self.remote_client_map:get(disconnected_guid)
			self.sample_scene.world_object.world:delete_entity(remote_self.parent_entity, nil)
			self.remote_client_map:remove(disconnected_guid)
			
			print("Player disconnected.")
		
		elseif message_type == protocol.message.STATE_UPDATE then
			local bsIn = self.received:get_bitstream()
			bsIn:IgnoreBytes(1)
				
			local this_body = self.sample_scene.player.body.physics.body
			this_body:SetTransform(Readb2Vec2(bsIn), 0)
			this_body:SetLinearVelocity(Readb2Vec2(bsIn))
	
			local new_states_num = ReadUshort(bsIn)		
				--print ("states: " .. new_states_num)
				--print ("size: " .. self.remote_client_map:size())
			for i=1, new_states_num do
				-- the notification about the new player might yet not have arrived
				local remote_client = self.remote_client_map:find(ReadRakNetGUID(bsIn))
				local new_position = Readb2Vec2(bsIn)
				local new_velocity = Readb2Vec2(bsIn)
				
				if remote_client.found then
					--print "found"
					local body = remote_client.value.parent_entity.physics.body
					body:SetTransform(new_position, 0)
					body:SetLinearVelocity(new_velocity)
				end
			end
			
		else
			print(protocol.message.NEW_PLAYER)
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

