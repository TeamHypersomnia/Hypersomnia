
dofile "hypersomnia\\scripts\\protocol.lua"

dofile "hypersomnia\\scripts\\reliable_channel.lua"

dofile "hypersomnia\\scripts\\components\\orientation.lua"
dofile "hypersomnia\\scripts\\components\\input_prediction.lua"
dofile "hypersomnia\\scripts\\components\\interpolation.lua"

dofile "hypersomnia\\scripts\\sync_modules\\modules.lua"
dofile "hypersomnia\\scripts\\sync_modules\\movement_sync.lua"
dofile "hypersomnia\\scripts\\sync_modules\\crosshair_sync.lua"

dofile "hypersomnia\\scripts\\systems\\protocol_system.lua"
dofile "hypersomnia\\scripts\\systems\\interpolation_system.lua"
dofile "hypersomnia\\scripts\\systems\\orientation_system.lua"
dofile "hypersomnia\\scripts\\systems\\client_system.lua"
dofile "hypersomnia\\scripts\\systems\\input_prediction_system.lua"
dofile "hypersomnia\\scripts\\systems\\synchronization_system.lua"

client_screen = inherits_from ()

function client_screen:constructor(camera_rect)
	self.sample_scene = scene_class:create()
	self.sample_scene:load_map("hypersomnia\\data\\maps\\sample_map.lua", "hypersomnia\\scripts\\loaders\\basic_map_loader.lua")
	
	self.sample_scene.world_camera.camera.screen_rect = camera_rect
	
	self.server = network_interface()
	
	self.server:connect(config_table.server_address, config_table.server_port)
	
	if config_table.simulate_lag ~= 0 then
		print "Simulating lag..."
		self.server:enable_lag(config_table.packet_loss, config_table.min_latency, config_table.jitter)
	end
	
	self.received = network_packet()
	
	-- entity system setup
	self.entity_system_instance = entity_system:create()
	
	self.entity_system_instance:register_messages {
		"network_message"
	}
	
	self.entity_system_instance:register_messages (protocol.message_names)
	
	self.systems = {}
	self.systems.client = client_system:create(self.server)
	self.systems.input_prediction = input_prediction_system:create(self.sample_scene.simulation_world)
	self.systems.synchronization = synchronization_system:create(self.sample_scene)
	self.systems.protocol = protocol_system:create(function(msg) self.systems.synchronization:handle_variable_message(msg) end)
	self.systems.interpolation = interpolation_system:create()
	self.systems.orientation = orientation_system:create()
	
	table.insert(self.sample_scene.world_object.substep_callbacks, function () 
		self.systems.client:substep()
		self.systems.input_prediction:substep()
	end)
	
	
	self.entity_system_instance:register_systems(self.systems)
end

function client_screen:loop()

	setlsys(self.sample_scene.world_object.render_system)
		
	-- handle networking
	local packet = self.received
	
	if (self.server:receive(self.received)) then
		local message_type = self.received:byte(0)
			
		if message_type == network_event.ID_CONNECTION_REQUEST_ACCEPTED then
			self.server_guid = self.received:guid()
			self.systems.client.server_guid = self.server_guid
			print("Our connection request has been accepted.");
		elseif message_type == network_event.ID_NO_FREE_INCOMING_CONNECTIONS then
			print("The server is full.\n")
		elseif message_type == network_event.ID_DISCONNECTION_NOTIFICATION then
			print("Server has disconnected.\n")
		elseif message_type == network_event.ID_CONNECTION_LOST then
			print("Server lost the connection.\n")
		elseif message_type == protocol.GAME_TRANSMISSION then
			self.entity_system_instance:post_table("network_message", {
				data = packet,
				channel = self.systems.client.net_channel 
			})
		end
	end
	
	self.systems.client:clear_unreliable()
	
	self.systems.protocol:handle_incoming_commands()
	
	self.systems.input_prediction:update()
	self.systems.interpolation:update()
	self.systems.orientation:update()
	
	--print "client tick"
	self.systems.client:send_all_data()
	
	--print "flush"
	self.entity_system_instance:flush_messages()

	--print "scene loop"
	self.sample_scene:loop()
end

function client_screen:close_connection()
	if self.server_guid ~= nil then
		self.server:close_connection(self.server_guid, send_priority.IMMEDIATE_PRIORITY)
	end
	self.server_guid = nil
end

