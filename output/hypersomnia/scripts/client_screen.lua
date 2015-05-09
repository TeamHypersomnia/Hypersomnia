dofile "hypersomnia\\scripts\\components\\orientation_component.lua"
dofile "hypersomnia\\scripts\\components\\input_prediction_component.lua"
dofile "hypersomnia\\scripts\\components\\interpolation_component.lua"
dofile "hypersomnia\\scripts\\components\\weapon_component.lua"
dofile "hypersomnia\\scripts\\components\\lifetime_component.lua"
dofile "hypersomnia\\scripts\\components\\health_component.lua"
dofile "hypersomnia\\scripts\\components\\wield_component.lua"
dofile "hypersomnia\\scripts\\components\\item_component.lua"
dofile "hypersomnia\\scripts\\components\\inventory_component.lua"
dofile "hypersomnia\\scripts\\components\\label_component.lua"
dofile "hypersomnia\\scripts\\components\\sound_component.lua"
dofile "hypersomnia\\scripts\\components\\light_component.lua"
dofile "hypersomnia\\scripts\\components\\particle_response_component.lua"

dofile "hypersomnia\\scripts\\sync_modules\\modules.lua"
dofile "hypersomnia\\scripts\\sync_modules\\movement_sync.lua"
dofile "hypersomnia\\scripts\\sync_modules\\health_sync.lua"
dofile "hypersomnia\\scripts\\sync_modules\\crosshair_sync.lua"
dofile "hypersomnia\\scripts\\sync_modules\\item_sync.lua"
dofile "hypersomnia\\scripts\\sync_modules\\gun_sync.lua"
dofile "hypersomnia\\scripts\\sync_modules\\label_sync.lua"

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
dofile "hypersomnia\\scripts\\systems\\inventory_system_shared.lua"

dofile "hypersomnia\\scripts\\systems\\wield_system.lua"
dofile "hypersomnia\\scripts\\systems\\item_system.lua"
dofile "hypersomnia\\scripts\\systems\\label_system.lua"

dofile "hypersomnia\\scripts\\systems\\melee_system.lua"

dofile "hypersomnia\\scripts\\systems\\health_system.lua"
dofile "hypersomnia\\scripts\\systems\\sound_system.lua"

dofile "hypersomnia\\scripts\\systems\\light_system.lua"

dofile "hypersomnia\\scripts\\chat_gui.lua"
dofile "hypersomnia\\scripts\\gui\\gui.lua"

client_screen = inherits_from ()


function client_screen:constructor(camera_rect)
	self.sample_scene = scene_class:create()
	self.sample_scene.owner_client_screen = self

	self.sample_scene:load_map("hypersomnia\\data\\maps\\cathedral2.lua", "hypersomnia\\scripts\\loaders\\basic_map_loader.lua",
	{
		kubasta = {
			filename = "hypersomnia/data/Kubasta.ttf", 
			size = 16,
			letters = " ABCDEFGHIJKLMNOPRSTUVWXYZQabcdefghijklmnoprstuvwxyzq0123456789.!@#$%^&*()_+-=[];'\\,./{}:\"|<>?"
		}
	}
	
	)
	
	self.sample_scene.world_camera.camera.screen_rect = camera_rect
	self.sample_scene.world_object.owner_client_screen = self
	
	self.server = network_interface()
	self.server:occasional_ping(true)
	
	self.server:connect(config_table.server_address, config_table.server_port)
	
	if config_table.simulate_lag ~= 0 then
		print "Simulating lag..."
		self.server:enable_lag(config_table.packet_loss, config_table.min_latency, config_table.jitter)
	end
	
	self.received = network_packet()
	
	-- entity system setup
	self.entity_system_instance = entity_system:create(function () 
		self.sample_scene.world_object:call_deletes() 
		self.sample_scene.simulation_world:call_deletes() 
	end)
	
	self.entity_system_instance:register_messages {
		"network_message",
		"shot_message",
		"item_wielder_change",
		
		"begin_swinging",
		"swing_hitcheck"
	}
	
	self.entity_system_instance:register_messages (protocol.message_names)
	
	self.systems = {}
	self.systems.client = client_system:create(self.server)
	self.systems.input_prediction = input_prediction_system:create(self.sample_scene.simulation_world)
	self.systems.replication = replication_system:create(self.sample_scene)
	self.systems.protocol = protocol_system:create(function(msg) return self.systems.replication:get_variable_message_size(msg) end,
	function (input_bs)
		
	end)
	
	self.systems.protocol.pack_loops = true

	self.systems.lifetime = lifetime_system:create(self.sample_scene.world_object)
	self.systems.interpolation = interpolation_system:create(self.sample_scene.simulation_world)
	self.systems.orientation = orientation_system:create()
	self.systems.weapon = weapon_system:create(self.sample_scene.world_object, self.sample_scene.world_object.physics_system)
	self.systems.bullet_creation = bullet_creation_system:create(self.sample_scene.world_object, self.sample_scene.world_camera, self.sample_scene.simulation_world)

	self.systems.health = health_system:create(self.sample_scene)
	
	self.systems.wield = wield_system:create()
	self.systems.item = item_system:create(self.sample_scene.world_object)
	self.systems.inventory = inventory_system:create(self.sample_scene)
	
	self.systems.label = label_system:create()
	
	self.systems.melee = melee_system:create(self.sample_scene.world_object)
	self.systems.sound = sound_system:create(self.sample_scene.world_object)
	self.systems.light = light_system:create(self.sample_scene.sprite_library["blank"])
	
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
	
	self.entity_system_instance:register_callbacks {
		item_wielder_change = function(msg) 
			self.systems.wield:handle_wielder_change(msg)
			self.systems.inventory:handle_picked_up_item(msg)
		end
	}
	

	create_weapons(self.sample_scene, true)
	
	self.my_gui = gui_class:create(camera_rect, self.sample_scene.world_object, self)
	
	sf_Listener_setDirection(0, 1, 0)
	sf_Listener_setGlobalVolume(100)
	sf_Listener_setPosition(0, 0, 0)
	
	--local ch_l = create_music("hypersomnia\\data\\music\\choir_l.ogg")
	--local ch_r = create_music("hypersomnia\\data\\music\\choir_r.ogg")
	--
	--self.sample_scene.choir_l = self.entity_system_instance:add_entity (components.create_components {
	--	cpp_entity = self.sample_scene.world_object:create_entity {
	--		transform = {
	--			pos = vec2(-1400 - 100, -2900)
	--		}
	--	},
	--	
	--	sound = {
	--		effect_type = components.sound.effect_types.MUSIC,
	--		music_object = ch_l
	--	}
	--})
	--
	--self.sample_scene.choir_l = self.entity_system_instance:add_entity (components.create_components {
	--	cpp_entity = self.sample_scene.world_object:create_entity {
	--		transform = {
	--			pos = vec2(-1400 + 100, -2900)
	--		}
	--	},
	--	
	--	sound = {
	--		effect_type = components.sound.effect_types.MUSIC,
	--		music_object = ch_r
	--	}
	--})
--
	--self.sample_scene.sprite_object_library.torso.white.walk.rifle["2"].size_multiplier = vec2(50, 50)

	--self.sample_scene.aaaa = self.entity_system_instance:add_entity (components.create_components {
--	--	cpp_entity = self.sample_scene.world_object:create_entity {
--	--		transform = {
--	--			pos = vec2(608, 448)
--	--		},
--
	--		render = {
	--			model = self.sample_scene.sprite_object_library.torso.white.walk.rifle["2"],
	--			layer = render_layers.CROSSHAIRS
	--		}
	--	}
	--})
	self.sample_scene.load_objects()
end

function client_screen:send(msg_bs)
	self.server:send(protocol.make_reliable_bs(msg_bs), send_priority.IMMEDIATE_PRIORITY, send_reliability.RELIABLE_ORDERED, 0, self.server_guid, false)
end

stabilitytimer = timer()
function client_screen:loop()
	--if self.sample_scene.player then
	--self.sample_scene.player.cpp_entity.transform.current.pos.x = self.sample_scene.player.cpp_entity.transform.current.pos.x + 50*stabilitytimer:extract_seconds()
	--end
	setlsys(self.sample_scene.world_object.render_system)
		
	-- handle networking
	local packet = self.received
	
	if (self.server:receive(self.received)) then
		local message_type = self.received:byte(0)
		local is_reliable_transmission = message_type == protocol.RELIABLE_TRANSMISSION
			
		if message_type == network_event.ID_CONNECTION_REQUEST_ACCEPTED then
			self.server_guid = self.received:guid()
			self.systems.client.server_guid = self.server_guid
			
			print("Our connection request has been accepted.");
			
			cprint "Our connection request has been accepted."
			
			self:send(protocol.write_msg("BEGIN_SESSION", {
				nickname = towchar_vec(config_table.nickname)
			}))
		elseif message_type == network_event.ID_NO_FREE_INCOMING_CONNECTIONS then
			print("The server is full.\n")
			cprint("The server is full.\n")
		elseif message_type == network_event.ID_DISCONNECTION_NOTIFICATION then
			print("Server has disconnected.\n")
			cprint("Server has disconnected.\n")
		elseif message_type == network_event.ID_CONNECTION_LOST then
			print("Server lost the connection.\n")
			cprint("Server lost the connection.\n")
		elseif message_type == protocol.GAME_TRANSMISSION or is_reliable_transmission then
			self.entity_system_instance:post_table("network_message", {
				data = packet,
				["is_reliable_transmission"] = is_reliable_transmission,
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

	-- it is valid to call this update now
	-- as all objects have been created
	self.systems.replication:update_object_states()
	
	self.systems.wield:receive_item_wieldings()
	
	
	self.systems.input_prediction:update()
	self.systems.interpolation:update()
	self.systems.orientation:update()
	
	self.systems.lifetime:update()
	self.systems.inventory:update()
	-- post-inventory pass update for prediction

	self.systems.weapon:handle_messages()
	self.systems.weapon:translate_shot_info_msgs()
	self.systems.weapon:update()
	self.systems.bullet_creation:update()
	self.systems.melee:process_swinging()
	
	-- hit info translation implies deleting some of the bullets so it is to be executed
	-- after their potential creation (bullet_creation:update())
	self.systems.lifetime:translate_hit_infos()
		
		
	self.systems.health:update()
	--self.systems.sound:update()
		
	
	cpp_world:process_all_systems()

	self.systems.replication:delete_objects()

	self.entity_system_instance:handle_removed_entities()
	
	handle_incoming_chat(self)
	
	self.entity_system_instance:flush_messages()	
	
	cpp_world:consume_events()
	cpp_world:render()

	self.sample_scene.world_camera.script:tick()
	--collectgarbage("collect")
end

function client_screen:close_connection()
	if self.server_guid ~= nil then
		self.server:close_connection(self.server_guid, send_priority.IMMEDIATE_PRIORITY)
	end
	self.server_guid = nil
end

