-- initialize utility scripts
dofile "hypersomnia\\scripts\\scene_class.lua"


-- initialize gameplay libraries
dofile "hypersomnia\\scripts\\game\\layers.lua"
dofile "hypersomnia\\scripts\\game\\camera.lua"


sample_scene = scene_class:create()
sample_scene:load_map_and_set_current("hypersomnia\\data\\maps\\sample_map", "hypersomnia\\scripts\\loaders\\basic_map_loader")