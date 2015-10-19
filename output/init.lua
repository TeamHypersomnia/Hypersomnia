global_logfile = io.open("client_logfile.log", "w")
transmission_log = io.open("client_transmission.log", "w")

dofile "config.lua"

ENGINE_DIRECTORY = "..\\..\\Augmentations\\scripts\\"
dofile (ENGINE_DIRECTORY .. "load_libraries.lua")

--setup_debugger()
-- enter the game
dofile "hypersomnia\\scripts\\start.lua"

transmission_log:close()
global_logfile:close()