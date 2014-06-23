protocol = {}

protocol.module_mappings = {
	"movement",
	"orientation"
}

protocol.messages = {
	-- client to server
	"COMMAND",
	
	-- server to client
	"INITIAL_STATE",
	
	"NEW_OBJECTS",
	"DELETE_OBJECT",
	
	-- used for guaranteed in-order delivery state changes
	"STATE_UPDATE"
	-- used for sequenced visuals like positions/velocities
	"STREAM_UPDATE"
	-- we must have these two types of updates because of differing transportation methods
}


protocol.intent_to_name = {
	[intent_message.MOVE_FORWARD] = "forward",
	[intent_message.MOVE_BACKWARD] = "backward",
	[intent_message.MOVE_LEFT] = "left",
	[intent_message.MOVE_RIGHT] = "right"
}

protocol.name_to_command = {
	["+forward"] = 1,
	["-forward"] = 2,
	["+backward"] = 3,
	["-backward"] = 4,
	["+left"] = 5,
	["-left"] = 6,
	["+right"] = 7,
	["-right"] = 8
}

protocol.name_to_intent = {}
protocol.command_to_name = {}

for k, v in pairs (name_to_command) do
	protocol.command_to_name[v] = k
end

for k, v in pairs (intent_to_name) do
	protocol.name_to_intent[v] = k
end

for i=1, #protocol.messages[i] do
	local new_table = {}

	new_table[protocol.messages[i]] = network_message.ID_USER_PACKET_ENUM + i
end	