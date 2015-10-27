xpcall(function () 
global_logfile = io.open("logs/client_logfile.log", "w")
transmission_log = io.open("logs/client_transmission.log", "w")

dofile "config.lua"

ENGINE_DIRECTORY = "..\\..\\Augmentations\\scripts\\"
dofile (ENGINE_DIRECTORY .. "load_libraries.lua")

--setup_debugger()
-- enter the game
dofile "hypersomnia\\scripts\\start.lua"

transmission_log:close()
global_logfile:close()
end, function(error_message) 
		print("xpcall failed:")
		local full_error = error_message .. debug.traceback()
		print(full_error)
		open_editor(full_error)
		if debug.post_traceback ~= nil then 
			--debug.post_traceback() 
			print("post traceback written.") 
		else
			print("post traceback not yet defined.") 
		end 
	end)