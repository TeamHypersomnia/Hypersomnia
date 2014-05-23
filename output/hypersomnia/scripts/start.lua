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
sample_scene:set_current()


print "Initialization is OK."
-- main loop

while true do
	if augmentations_main_loop_callback ~= nil then
		local result = augmentations_main_loop_callback()
		if result ~= nil and result ~= 0 then break end
	end
	
	if call_once_after_loop ~= nil then
		call_once_after_loop()
		call_once_after_loop = nil
	end
end

