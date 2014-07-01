protocol = {}

-- enables modules to be read/written correctly in order
protocol.module_mappings = {
	"movement",
	"orientation"
}

protocol.GAME_TRANSMISSION = network_event.ID_USER_PACKET_ENUM + 1

protocol.messages = {
	-- client to server
	"COMMAND",
	
	-- bidirectional
	"CLIENT_PREDICTION",
	
	-- server to client
	"ASSIGN_SYNC_ID",
	
	-- used for guaranteed in-order delivery state changes
	"STATE_UPDATE",
	-- used for sequenced visuals like positions/velocities
	"STREAM_UPDATE",
	-- we must have these two types of updates because of differing transportation methods
	
	"DELETE_OBJECT"
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

for k, v in pairs (protocol.name_to_command) do
	protocol.command_to_name[v] = k
end

for k, v in pairs (protocol.intent_to_name) do
	protocol.name_to_intent[v] = k
end

local new_msg_table = {}
	
for i=1, #protocol.messages do
	new_msg_table[protocol.messages[i]] = i
end	

protocol.messages = new_msg_table