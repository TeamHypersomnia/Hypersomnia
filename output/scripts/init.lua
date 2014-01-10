--package.path = "../?.lua;" .. package.path
--package.path = package.path
table.inspect = require("inspect")

function debug.my_traceback() 
	local outstr = "\n"
	
	local globals_str = "\n"
	
	globals_str = globals_str .. "Globals:\n"
	
	for k,v in pairs(_G) do
		globals_str = globals_str .. k .. ":\n" .. table.inspect(v) .. "\n\n"
	end
	
	local level_idx = 2
	while true do
		outstr = outstr .. "level " .. level_idx .. ": "
		
		local variables = {}
		local idx = 1
		while true do
			local ln, lv = debug.getlocal(level_idx, idx)
			if ln ~= nil then
			variables[ln] = lv
			else
			break
			end
			idx = 1 + idx
		end
		
		outstr = outstr .. table.inspect(variables) .. "\n"
		
		level_idx = level_idx + 1
		if debug.getinfo(level_idx) == nil then break end
	end
	
	outstr = outstr .. "\n" .. debug.traceback() 
	
	
	local file = io.open("error_message.txt", "w")
	file:write(outstr .. globals_str)
	file:close()
	
	return outstr
end


--local f, vrf = xpcall(function() 

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

local CURRENT_SCENE = "scripts\\sample_scenes\\soldier_ai.lua"
npc_script_file = open_script "scripts\\sample_scenes\\npc.lua"
soldier_tree_file = open_script "scripts\\sample_scenes\\soldier_tree.lua"
steering_file = open_script "scripts\\sample_scenes\\steering.lua"
map_file = open_script "scripts\\sample_scenes\\map.lua"
weapons_file = open_script "scripts\\sample_scenes\\weapons.lua"
player_file = open_script "scripts\\sample_scenes\\player.lua"

common = 						open_script "scripts\\common.lua" 
textures = 						open_script "scripts\\resources\\textures.lua"
resource_creation_util = 		open_script "scripts\\resource_creation_util.lua"
entity_creation_util = 			open_script "scripts\\entity_creation_util.lua" 

layers = 						open_script "scripts\\resources\\layers.lua"
animations = 					open_script "scripts\\resources\\animations.lua"
particle_effects = 				open_script "scripts\\resources\\particle_effects.lua"
settings = 						open_script "scripts\\settings.lua"
entities = 						open_script (CURRENT_SCENE)

call_on_modification(common, 				{ common, layers, entity_creation_util, resource_creation_util, textures, animations, particle_effects, entities }) 
call_on_modification(textures, 				{ textures, animations, particle_effects, entities })
call_on_modification(resource_creation_util,{ resource_creation_util, entity_creation_util, textures, animations, particle_effects, entities })
call_on_modification(entity_creation_util, 	{ entity_creation_util, entities } )

call_on_modification(layers, 				{ layers, animations, particle_effects, entities })
call_on_modification(animations, 			{ animations, particle_effects, entities })
call_on_modification(particle_effects, 		{ particle_effects, entities })
call_on_modification(entities, 				{ entities })
call_on_modification(settings, 				{ settings })

call_on_modification(npc_script_file, 				{ entities })
call_on_modification(soldier_tree_file, 				{ entities })
call_on_modification(steering_file, 				{ entities })
call_on_modification(map_file, 				{ entities })
call_on_modification(weapons_file, 				{ entities })
call_on_modification(player_file, 				{ entities })

dofile "scripts\\common.lua"
dofile "scripts\\entity_creation_util.lua" 
dofile "scripts\\resource_creation_util.lua"
dofile "scripts\\resources\\layers.lua"
dofile "scripts\\resources\\textures.lua"
dofile "scripts\\resources\\animations.lua"
dofile "scripts\\resources\\particle_effects.lua"
dofile "scripts\\settings.lua"
dofile (CURRENT_SCENE)

commands = script()
commands:associate_filename("scripts\\commands.lua")
commands.reload_scene_when_modified = false
settings.reload_scene_when_modified = false

call_on_modification(commands, {commands})

function count_all(f)
	local seen = {}
	local count_table
	count_table = function(t)
		if seen[t] then return end
		f(t)
		seen[t] = true
		for k,v in pairs(t) do
			if type(v) == "table" then
				count_table(v)
			elseif type(v) == "userdata" then
				f(v)
			end
		end
	end
	count_table(_G)
end

function all_num()
	local cnt = 0
	
	count_all(function() cnt = cnt + 1 end)
	
	print(cnt)
end

--end, 
--debug.my_traceback)
--print (f,vrf)

