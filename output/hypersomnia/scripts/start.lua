-- initialize gameplay libraries
dofile "hypersomnia\\scripts\\game\\input.lua"
dofile "hypersomnia\\scripts\\game\\layers.lua"
dofile "hypersomnia\\scripts\\game\\camera.lua"

-- archetypes
dofile "hypersomnia\\scripts\\archetypes\\basic_player.lua"

-- resource handling utilities
dofile "hypersomnia\\scripts\\resources\\animations.lua"

-- main loop

local file_watcher_object = file_watcher()
file_watcher_object:add_directory("hypersomnia\\scripts", false)

sample_scene = scene_class:create()
sample_scene:load_map("hypersomnia\\data\\maps\\sample_map", "hypersomnia\\scripts\\loaders\\basic_map_loader")

sample_scene2 = scene_class:create()
sample_scene2:load_map("hypersomnia\\data\\maps\\sample_map", "hypersomnia\\scripts\\loaders\\basic_map_loader")

sample_scene3 = scene_class:create()
sample_scene3:load_map("hypersomnia\\data\\maps\\sample_map", "hypersomnia\\scripts\\loaders\\basic_map_loader")

sample_scene4 = scene_class:create()
sample_scene4:load_map("hypersomnia\\data\\maps\\sample_map", "hypersomnia\\scripts\\loaders\\basic_map_loader")

sample_scene.world_camera.camera.screen_rect = rect_xywh(config_table.resolution_w/2, config_table.resolution_h/2, config_table.resolution_w/2, config_table.resolution_h/2)
sample_scene2.world_camera.camera.screen_rect = rect_xywh(0, 0, config_table.resolution_w/2, config_table.resolution_h/2)
sample_scene3.world_camera.camera.screen_rect = rect_xywh(config_table.resolution_w/2, 0, config_table.resolution_w/2, config_table.resolution_h/2)
sample_scene4.world_camera.camera.screen_rect = rect_xywh(0, config_table.resolution_h/2, config_table.resolution_w/2, config_table.resolution_h/2)

while true do
	GL.glClear(GL.GL_COLOR_BUFFER_BIT)
				
	sample_scene:loop()
	sample_scene2:loop()
	sample_scene3:loop()
	sample_scene4:loop()
	
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
	       
	local files_to_reload = file_watcher_object:get_modified_files()
	   
	for i=0, files_to_reload:size()-1 do 
	print(i)
		if files_to_reload:at(i) == "hypersomnia\\scripts\\commands.lua" then
			dofile "hypersomnia\\scripts\\commands.lua"
		end
	end
	
	global_gl_window:swap_buffers()
end

