network_message.ID_INITIAL_STATE = network_message.ID_USER_PACKET_ENUM + 1
network_message.ID_MOVEMENT = network_message.ID_USER_PACKET_ENUM + 2
network_message.ID_NEW_PLAYER = network_message.ID_USER_PACKET_ENUM + 3
network_message.ID_PLAYER_DISCONNECTED = network_message.ID_USER_PACKET_ENUM + 4
network_message.ID_COMMAND = network_message.ID_USER_PACKET_ENUM + 5
network_message.ID_STATE_UPDATE = network_message.ID_USER_PACKET_ENUM + 6

intent_to_name = {
	[intent_message.MOVE_FORWARD] = "forward",
	[intent_message.MOVE_BACKWARD] = "backward",
	[intent_message.MOVE_LEFT] = "left",
	[intent_message.MOVE_RIGHT] = "right"
}

name_to_command = {
	["+forward"] = 1,
	["-forward"] = 2,
	["+backward"] = 3,
	["-backward"] = 4,
	["+left"] = 5,
	["-left"] = 6,
	["+right"] = 7,
	["-right"] = 8
}

name_to_intent = {}
command_to_name = {}

for k, v in pairs (name_to_command) do
	command_to_name[v] = k
end

for k, v in pairs (intent_to_name) do
	name_to_intent[v] = k
end