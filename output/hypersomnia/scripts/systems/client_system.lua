client_system = inherits_from (processing_system)

function client_system:constructor(network) 
	self.network = network
		
	-- setup reliable channel
	self.net_channel = reliable_channel_wrapper:create()
	
	self.my_sync_id = nil
	
	processing_system.constructor(self)
end

function client_system:get_required_components()
	return { "client" }
end

function client_system:handle_incoming_commands()
	-- pre-clear the buffer
	self.net_channel.unreliable_buf:Reset()
	
	local msgs = self.owner_entity_system.messages["network_message"]
	
	for i=1, #msgs do
		local data = msgs[i].data
		
		local input_bs = data:get_bitstream()
	
		-- if there are some commands or streams to be read from the server
		if self.net_channel:recv(input_bs) ~= receive_result.NOTHING_RECEIVED then
			self.owner_entity_system:post( server_commands:create { 
				bitstream = input_bs
			})
		end
	end
end

function client_system:update_tick()
	-- write all pending reliable and unreliable data
	local output_bs = self.net_channel:send()
	
	if self.server_guid ~= nil and output_bs:size() > 0 then
		print("Sending: \n\n" .. auto_string_indent(output_bs.content) .. "\n\n")
		self.network:send(output_bs, send_priority.IMMEDIATE_PRIORITY, send_reliability.UNRELIABLE, 0, self.server_guid, false)
	end
end