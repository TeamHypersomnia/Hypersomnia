dofile "hypersomnia\\scripts\\components\\orientation.lua"
dofile "hypersomnia\\scripts\\components\\input_prediction.lua"
dofile "hypersomnia\\scripts\\components\\interpolation.lua"
dofile "hypersomnia\\scripts\\components\\weapon.lua"
dofile "hypersomnia\\scripts\\components\\lifetime.lua"
dofile "hypersomnia\\scripts\\components\\health.lua"
dofile "hypersomnia\\scripts\\components\\wield.lua"
dofile "hypersomnia\\scripts\\components\\item.lua"
dofile "hypersomnia\\scripts\\components\\inventory.lua"

dofile "hypersomnia\\scripts\\sync_modules\\modules.lua"
dofile "hypersomnia\\scripts\\sync_modules\\movement_sync.lua"
dofile "hypersomnia\\scripts\\sync_modules\\health_sync.lua"
dofile "hypersomnia\\scripts\\sync_modules\\crosshair_sync.lua"
dofile "hypersomnia\\scripts\\sync_modules\\item_sync.lua"
dofile "hypersomnia\\scripts\\sync_modules\\gun_sync.lua"

dofile "hypersomnia\\scripts\\systems\\protocol_system.lua"
dofile "hypersomnia\\scripts\\systems\\interpolation_system.lua"
dofile "hypersomnia\\scripts\\systems\\orientation_system.lua"
dofile "hypersomnia\\scripts\\systems\\client_system.lua"
dofile "hypersomnia\\scripts\\systems\\input_prediction_system.lua"
dofile "hypersomnia\\scripts\\systems\\replication_system.lua"
dofile "hypersomnia\\scripts\\systems\\weapon_system.lua"
dofile "hypersomnia\\scripts\\systems\\bullet_creation_system.lua"
dofile "hypersomnia\\scripts\\systems\\lifetime_system.lua"
dofile "hypersomnia\\scripts\\systems\\inventory_system.lua"

dofile "hypersomnia\\scripts\\systems\\wield_system.lua"
dofile "hypersomnia\\scripts\\systems\\item_system.lua"

dofile "hypersomnia\\scripts\\systems\\health_system.lua"

client_screen = inherits_from ()

function client_screen:constructor(camera_rect)
	self.sample_scene = scene_class:create()
	self.sample_scene:load_map("hypersomnia\\data\\maps\\sample_map.lua", "hypersomnia\\scripts\\loaders\\basic_map_loader.lua")
	
	self.sample_scene.world_camera.camera.screen_rect = camera_rect
	
	self.server = network_interface()
	self.server:occasional_ping(true)
	
	self.server:connect(config_table.server_address, config_table.server_port)
	
	if config_table.simulate_lag ~= 0 then
		print "Simulating lag..."
		self.server:enable_lag(config_table.packet_loss, config_table.min_latency, config_table.jitter)
	end
	
	self.received = network_packet()
	
	-- entity system setup
	self.entity_system_instance = entity_system:create(function () self.sample_scene.world_object:call_deletes() end)
	
	self.entity_system_instance:register_messages {
		"network_message",
		"shot_message",
		"item_wielder_change",
		
		"item_selection",
		"item_holster"
	}
	
	self.entity_system_instance:register_messages (protocol.message_names)
	
	self.systems = {}
	self.systems.client = client_system:create(self.server)
	self.systems.input_prediction = input_prediction_system:create(self.sample_scene.simulation_world)
	self.systems.replication = replication_system:create(self.sample_scene)
	self.systems.protocol = protocol_system:create(function(msg) return self.systems.replication:get_variable_message_size(msg) end,
	function (input_bs)
		
	end)
	
	self.systems.protocol.pack_updates = true

	self.systems.lifetime = lifetime_system:create(self.sample_scene.world_object)
	self.systems.interpolation = interpolation_system:create()
	self.systems.orientation = orientation_system:create()
	self.systems.weapon = weapon_system:create(self.sample_scene.world_object, self.sample_scene.world_object.physics_system)
	self.systems.bullet_creation = bullet_creation_system:create(self.sample_scene.world_object, self.sample_scene.world_camera, self.sample_scene.simulation_world)
	
	self.systems.health = health_system:create(self.sample_scene)
	
	self.systems.wield = wield_system:create()
	self.systems.item = item_system:create(self.sample_scene.world_object)
	self.systems.inventory = inventory_system:create(self.sample_scene)
	
	table.insert(self.sample_scene.world_object.prestep_callbacks, function (dt)
		self.systems.input_prediction:substep()
		
		self.systems.client:send_all_data()
		self.systems.client:substep()
	end)
	
	table.insert(self.sample_scene.world_object.poststep_callbacks, function (dt)
		self.systems.lifetime:poststep()
		
		self.entity_system_instance:handle_removed_entities()
	end)
	
	self.entity_system_instance:register_systems(self.systems)
	
	create_weapons(self.sample_scene, true)
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
	
	local cpp_world = self.sample_scene.world_object
	
	cpp_world:handle_input()
	cpp_world:handle_physics()
	
	self.systems.protocol:handle_incoming_commands()
	self.systems.protocol:unpack_oldest_loop()
	
	self.systems.replication:create_new_objects()

	-- there's always one STATE_UPDATE per LOOP_SEPARATOR so it is valid to call this update now
	-- as all objects have been created
	self.systems.replication:update_object_states()
	
	self.systems.wield:receive_item_wieldings()
	self.systems.wield:update()
	
	self.systems.weapon:handle_messages()
	
	self.systems.input_prediction:update()
	self.systems.interpolation:update()
	self.systems.orientation:update()
	
	self.systems.weapon:translate_shot_info_msgs()
	self.systems.weapon:update()
	
	self.systems.lifetime:update()
	self.systems.inventory:handle_picked_up_items()
	self.systems.inventory:update()
	self.systems.inventory:handle_item_selections()
	-- post-inventory pass update for prediction
	self.systems.wield:update()

	self.systems.bullet_creation:update()
	
	-- hit info translation implies deleting some of the bullets so it is to be executed
	-- after their potential creation (bullet_creation:update())
	self.systems.lifetime:translate_hit_infos()
		
		
	self.systems.health:update()
		
	
	cpp_world:process_all_systems()

	self.systems.replication:delete_objects()

	self.entity_system_instance:handle_removed_entities()
	
	self.entity_system_instance:flush_messages()	
	
	cpp_world:consume_events()
	cpp_world:render()
end

function client_screen:close_connection()
	if self.server_guid ~= nil then
		self.server:close_connection(self.server_guid, send_priority.IMMEDIATE_PRIORITY)
	end
	self.server_guid = nil
end

