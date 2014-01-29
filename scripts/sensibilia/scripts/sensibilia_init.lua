script_reloader:add_directory ("sensibilia\\scripts", true)

textures = 						open_script "sensibilia\\scripts\\resources\\textures.lua"

layers = 						open_script "sensibilia\\scripts\\resources\\layers.lua"

reloader_script = open_script "sensibilia\\scripts\\sensibilia.lua"

--animations = 					open_script "sensibilia\\scripts\\resources\\animations.lua"
--particle_effects = 				open_script "sensibilia\\scripts\\resources\\particle_effects.lua"

--npc_script_file = open_script "hp\\scripts\\sample_scenes\\npc.lua"
--soldier_tree_file = open_script "hp\\scripts\\sample_scenes\\soldier_tree.lua"
--steering_file = open_script "hp\\scripts\\sample_scenes\\steering.lua"
--map_file = open_script "hp\\scripts\\sample_scenes\\map.lua"
--weapons_file = open_script "hp\\scripts\\sample_scenes\\weapons.lua"
--player_file = open_script "hp\\scripts\\sample_scenes\\player.lua"

--call_on_modification(textures, 				{ textures, animations, particle_effects, entities })
--call_on_modification(layers, 				{ layers, animations, particle_effects, entities })
--call_on_modification(animations, 			{ animations, particle_effects, entities })
--call_on_modification(particle_effects, 		{ particle_effects, entities })
--
--call_on_modification(npc_script_file, 				{ entities })
--call_on_modification(soldier_tree_file, 				{ entities })
--call_on_modification(steering_file, 				{ entities })
--call_on_modification(map_file, 				{ entities })
--call_on_modification(weapons_file, 				{ entities })
--call_on_modification(player_file, 				{ entities })
--
--dofile "hp\\scripts\\resources\\layers.lua"
--dofile "hp\\scripts\\resources\\textures.lua"
--dofile "hp\\scripts\\resources\\animations.lua"
--dofile "hp\\scripts\\resources\\particle_effects.lua"
--
--reloader_script = open_script "hp\\scripts\\sample_scenes\\soldier_ai.lua"
--dofile "hp\\scripts\\sample_scenes\\soldier_ai.lua"

call_on_modification( textures, { textures, reloader_script  } )
call_on_modification( layers, { layers, reloader_script  } )
call_on_modification( reloader_script, { reloader_script } )


dofile "sensibilia\\scripts\\resources\\layers.lua"
dofile "sensibilia\\scripts\\resources\\textures.lua"


dofile "sensibilia\\scripts\\sensibilia.lua"
