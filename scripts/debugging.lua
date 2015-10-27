function debug.my_breakpoint()
	print(debug.my_traceback())
	debugger_break()
end

function debug.get_stack_contents()
	local outstr = "\n"
	
	local globals_str = "\n"
	
	--globals_str = globals_str .. "Globals:\n"
	--
	--for k,v in pairs(_G) do
	--	globals_str = globals_str .. k .. ":\n" .. table.inspect(v) .. "\n\n"
	--end
	
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
	
	return debug.traceback() .. outstr .. "\n" .. debug.traceback(), globals_str
end


function debug.pre_traceback()
	return debug.traceback()
end

function debug.post_traceback() 
	local outstr, globals_str = debug.get_stack_contents()
	
	local file = io.open("logs/error_message.log", "w")
	file:write(outstr .. globals_str)
	file:close()
	
	if protocol ~= nil then
		if protocol.LAST_READ_BITSTREAM ~= nil then
			print (protocol.LAST_READ_BITSTREAM.read_report)
		end 
	end
end

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

table.inspect = dofile (ENGINE_DIRECTORY .. "inspect.lua")

function setup_debugger()
	package.path = package.path .. ";C:/Users/Anon/Downloads/ZeroBraneStudio/lualibs/?/?.lua" .. ";C:/Users/Anon/Downloads/ZeroBraneStudio/lualibs/?.lua"
	package.cpath = package.cpath .. ";C:/Users/Anon/Downloads/ZeroBraneStudio/bin/clibs52/?.dll" .. ";C:/Users/Anon/Downloads/ZeroBraneStudio/bin/?.dll";
	
	local old_tostr = tostring
	tostring = function(input)
		if type(input) == 'userdata' then
			return table.inspect(input)
		else
			return old_tostr(input)
		end
	end
	
	require("mobdebug").start()
end