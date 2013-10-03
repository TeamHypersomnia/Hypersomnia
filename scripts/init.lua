function call_on_modification(thescript, entries) 
	for i = 1, #entries do
		thescript:add_reload_dependant(entries[i])
	end
end

function open_script(filename) 
	local my_script = script()
	my_script:associate_filename(filename)
	return my_script
end

common = 						open_script "scripts\\common.lua" 
textures = 						open_script "scripts\\resources\\textures.lua"
entity_creation_util = 			open_script "scripts\\entity_creation_util.lua" 
resource_creation_util = 		open_script "scripts\\resource_creation_util.lua"

layers = 						open_script "scripts\\resources\\layers.lua"
animations = 					open_script "scripts\\resources\\animations.lua"
particle_effects = 				open_script "scripts\\resources\\particle_effects.lua"
entities = 						open_script "scripts\\sample_scene\\entities.lua"

call_on_modification(common, 				{ common, entity_creation_util, resource_creation_util, textures, animations, particle_effects, entities }) 
call_on_modification(textures, 				{ textures, animations, particle_effects, entities })
call_on_modification(entity_creation_util, 	{ entity_creation_util, entities } )
call_on_modification(resource_creation_util,{ resource_creation_util, textures, animations, particle_effects, entities })

call_on_modification(layers, 				{ layers, animations, particle_effects, entities })
call_on_modification(animations, 			{ animations, particle_effects, entities })
call_on_modification(particle_effects, 		{ particle_effects, entities })
call_on_modification(entities, 				{ entities })

dofile "scripts\\common.lua" 
dofile "scripts\\entity_creation_util.lua" 
dofile "scripts\\resource_creation_util.lua"
dofile "scripts\\resources\\layers.lua"
dofile "scripts\\resources\\textures.lua"
dofile "scripts\\resources\\animations.lua"
dofile "scripts\\resources\\particle_effects.lua"
dofile "scripts\\sample_scene\\entities.lua"

commands = script()
commands:associate_filename("scripts\\commands.lua")
commands.reload_scene_when_modified = false

call_on_modification(commands, {commands})
