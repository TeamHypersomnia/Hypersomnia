table.inspect = dofile (ENGINE_DIRECTORY .. "inspect.lua")

function debug.my_breakpoint()
	print(debug.my_traceback())
	debugger_break()
end

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
