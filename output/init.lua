dofile "config.lua"

ENGINE_DIRECTORY = "..\\..\\Augmentations\\scripts\\"
dofile (ENGINE_DIRECTORY .. "load_libraries.lua")

client = network_interface()
client:connect("127.0.0.1", 37017)

received = network_packet()

network_message.ID_GAME_MESSAGE_1 = network_message.ID_USER_PACKET_ENUM + 1

-- enter the game
dofile "hypersomnia\\scripts\\start.lua"