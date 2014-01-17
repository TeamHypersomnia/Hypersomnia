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

script_reloader:add_directory ("engine", true)

settings = 						open_script "engine\\settings.lua"
common = 						open_script "engine\\common.lua" 
resource_creation_util = 		open_script "engine\\resource_creation_util.lua"
entity_creation_util = 			open_script "engine\\entity_creation_util.lua" 

local CURRENT_SCENE = "hp\\scripts\\hp_init.lua"
entities = 						open_script (CURRENT_SCENE)

call_on_modification(common, 				{ common, layers, entity_creation_util, resource_creation_util, textures, animations, particle_effects, entities }) 
call_on_modification(resource_creation_util,{ resource_creation_util, entity_creation_util, textures, animations, particle_effects, entities })
call_on_modification(entity_creation_util, 	{ entity_creation_util, entities } )

call_on_modification(entities, 				{ entities })
call_on_modification(settings, 				{ settings })

dofile "engine\\common.lua"
dofile "engine\\entity_creation_util.lua" 
dofile "engine\\resource_creation_util.lua"
dofile "engine\\settings.lua"
dofile (CURRENT_SCENE)

commands = script()
commands:associate_filename("engine\\commands.lua")
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