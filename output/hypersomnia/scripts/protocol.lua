protocol = {}

-- enables modules to be read/written correctly in order
protocol.module_mappings = {
	"movement",
	"crosshair",
	"orientation"
}

protocol.message_by_id = {
	{
		name = "INPUT_SNAPSHOT",
		data = {
			"Uint", "at_step", 
			"Bit", 	"moving_left",
			"Bit", 	"moving_right",
			"Bit", 	"moving_forward",
			"Bit", 	"moving_backward"
		}
	},
	{
		name = "CROSSHAIR_SNAPSHOT",
		data = {
			"Vec2", "position"
		}
	},
	{
		name = "CURRENT_STEP",
		data = {
			"Uint", "at_step"
		}
	},
	{
		name = "ASSIGN_SYNC_ID",
		data = {
			"Ushort", "sync_id"
		},
		read_immediately = true
	},
	{
		name = "STATE_UPDATE",
		data = {
			"Ushort", "object_count"
		},
		read_immediately = true
	},
	{
		name = "STREAM_UPDATE",
		data = {
			"Ushort", "object_count"
		},
		read_immediately = true
	},
	
	{
		name = "DELETE_OBJECT",
		data = {
			"Ushort", "removed_id"
		},
		read_immediately = true
	}
}

-- internals
protocol.GAME_TRANSMISSION = network_event.ID_USER_PACKET_ENUM + 1

protocol.messages = {}
protocol.message_names = {}
protocol.id_by_message = {}

for i=1, #protocol.message_by_id do
	local name = protocol.message_by_id[i].name
	protocol.messages[name] = protocol.message_by_id[i]
	table.insert(protocol.message_names, name)
	protocol.id_by_message[name] = i
end	

protocol.write_sig = function(sig, entry, out_bs)
	for i=1, (#sig/2) do
		local var_type = sig[i*2-1]
		local var_name = sig[i*2]
		
		out_bs:name_property(var_name)
		
		if var_type == "Bit" then
			entry[var_name] = entry[var_name] > 0
		end
		out_bs["Write" .. var_type](out_bs, entry[var_name])
	end
end

protocol.write_msg = function(name, entry)
	local out_bs = BitStream()
	out_bs:name_property(name)
	out_bs:WriteByte(protocol.id_by_message[name])
	
	local data = protocol.messages[name].data
	
	protocol.write_sig(data, entry, out_bs)
	
	return out_bs
end


protocol.read_sig = function(sig, out_entry, in_bs)
	for i=1, (#sig/2) do
		local var_type = sig[i*2-1]
		local var_name = sig[i*2]
		
		in_bs:name_property(var_name)
		
		out_entry[var_name] = in_bs["Read" .. var_type](in_bs)
		
		if var_type == "Bit" then
			out_entry[var_name] = bool2int(out_entry[var_name])
		end
	end
end

protocol.read_msg = function(in_bs, out_table)
	local out_entry = out_table
	
	if out_entry == nil then
		out_entry = {}
	end
	
	in_bs:name_property("message_type")
	local message_type = in_bs:ReadByte()
	
	out_entry.info = protocol.message_by_id[message_type]
	out_entry.data = {}
	
	local data = protocol.message_by_id[message_type].data
	
	protocol.read_sig(data, out_entry.data, in_bs)

	return out_entry
end








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
