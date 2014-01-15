script_reloader:add_directory ("hp\\scripts", true)

textures = 						open_script "hp\\scripts\\resources\\textures.lua"

layers = 						open_script "hp\\scripts\\resources\\layers.lua"
animations = 					open_script "hp\\scripts\\resources\\animations.lua"
particle_effects = 				open_script "hp\\scripts\\resources\\particle_effects.lua"

npc_script_file = open_script "hp\\scripts\\sample_scenes\\npc.lua"
soldier_tree_file = open_script "hp\\scripts\\sample_scenes\\soldier_tree.lua"
steering_file = open_script "hp\\scripts\\sample_scenes\\steering.lua"
map_file = open_script "hp\\scripts\\sample_scenes\\map.lua"
weapons_file = open_script "hp\\scripts\\sample_scenes\\weapons.lua"
player_file = open_script "hp\\scripts\\sample_scenes\\player.lua"

reloader_script = open_script "hp\\scripts\\sample_scenes\\soldier_ai.lua"

call_on_modification(textures, 				{ textures, animations, particle_effects, reloader_script })
call_on_modification(layers, 				{ layers, animations, particle_effects, reloader_script })
call_on_modification(animations, 			{ animations, particle_effects, reloader_script })
call_on_modification(particle_effects, 		{ particle_effects, reloader_script })

call_on_modification(npc_script_file, 				{ reloader_script })
call_on_modification(soldier_tree_file, 				{ reloader_script })
call_on_modification(steering_file, 				{ reloader_script })
call_on_modification(map_file, 				{ reloader_script })
call_on_modification(weapons_file, 				{ reloader_script })
call_on_modification(player_file, 				{ reloader_script })


call_on_modification(reloader_script, 				{ reloader_script })

dofile "hp\\scripts\\resources\\layers.lua"
dofile "hp\\scripts\\resources\\textures.lua"
dofile "hp\\scripts\\resources\\animations.lua"
dofile "hp\\scripts\\resources\\particle_effects.lua"

dofile "hp\\scripts\\sample_scenes\\soldier_ai.lua"

