-- initialize gameplay libraries
dofile "hypersomnia\\scripts\\game\\input.lua"
dofile "hypersomnia\\scripts\\game\\layers.lua"
dofile "hypersomnia\\scripts\\game\\filters.lua"
dofile "hypersomnia\\scripts\\game\\camera.lua"

dofile "hypersomnia\\scripts\\game\\weapons.lua"

-- network defines
dofile "hypersomnia\\scripts\\protocol.lua"

dofile "hypersomnia\\scripts\\reliable_channel.lua"

-- archetypes
dofile "hypersomnia\\scripts\\archetypes\\archetype_library.lua"
dofile "hypersomnia\\scripts\\archetypes\\controlled_player.lua"
dofile "hypersomnia\\scripts\\archetypes\\remote_player.lua"
dofile "hypersomnia\\scripts\\archetypes\\gun.lua"
dofile "hypersomnia\\scripts\\archetypes\\inventory.lua"

-- resource handling utilities
dofile "hypersomnia\\scripts\\resources\\animations.lua"
dofile "hypersomnia\\scripts\\resources\\particle_effects.lua"

dofile "hypersomnia\\scripts\\client_screen.lua"

-- main loop

local file_watcher_object = file_watcher()
file_watcher_object:add_directory("hypersomnia\\scripts", false)

if config_table.multiple_clients_view ~= 0 then
	client_scenes = {
		--client_screen:create(rect_xywh(config_table.resolution_w/2, config_table.resolution_h/2, config_table.resolution_w/2, config_table.resolution_h/2)),
		client_screen:create(rect_xywh(0, 0, config_table.resolution_w/2, config_table.resolution_h/2)),
		client_screen:create(rect_xywh(config_table.resolution_w/2, 0, config_table.resolution_w/2, config_table.resolution_h/2)),
		--client_screen:create(rect_xywh(0, config_table.resolution_h/2, config_table.resolution_w/2, config_table.resolution_h/2))
	}
else	
	client_scenes = {
		client_screen:create(rect_xywh(0, 0, config_table.resolution_w, config_table.resolution_h))
	}
end


function set_active_client(which)
	if config_table.multiple_clients_view == 0 then
		which = 1
	end
	
	for i=1, #client_scenes do
		client_scenes[i].sample_scene.world_object.input_system:clear_contexts()
	end
	
	client_scenes[which].sample_scene.world_object.input_system:add_context(main_input_context)
end

set_active_client(1)

SHOULD_QUIT_FLAG = false

while not SHOULD_QUIT_FLAG do
	GL.glClear(GL.GL_COLOR_BUFFER_BIT)
	
	for i=1, #client_scenes do
		CURRENT_CLIENT_NUMBER = i
		client_scenes[i]:loop()
	end
	
	if call_once_after_loop ~= nil then
		call_once_after_loop()
		call_once_after_loop = nil
	end
	       
	local files_to_reload = file_watcher_object:get_modified_files()
	   
	for i=0, files_to_reload:size()-1 do 
		if files_to_reload:at(i) == "hypersomnia\\scripts\\commands.lua" then
			dofile "hypersomnia\\scripts\\commands.lua"
		end
	end
	
	global_gl_window:swap_buffers()
end


for i=1, #client_scenes do
	client_scenes[i]:close_connection()
end

