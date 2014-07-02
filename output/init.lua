
global_logfile = io.open("client_logfile.txt", "w")

dofile "config.lua"

ENGINE_DIRECTORY = "..\\..\\Augmentations\\scripts\\"
dofile (ENGINE_DIRECTORY .. "load_libraries.lua")

-- enter the game
dofile "hypersomnia\\scripts\\start.lua"


global_logfile:close()