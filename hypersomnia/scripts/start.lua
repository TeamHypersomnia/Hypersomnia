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