protocol_system = inherits_from (processing_system)

function protocol_system:constructor(variable_message_callback)
	self.variable_message_callback = variable_message_callback
	processing_system.constructor(self)
end

function protocol_system:handle_incoming_commands()
	-- pre-clear the buffer
	local msgs = self.owner_entity_system.messages["network_message"]
	
	for i=1, #msgs do
		local input_bs = msgs[i].data:get_bitstream()
	
		local result = msgs[i].channel:recv(input_bs)
		
		if result ~= receive_result.NOTHING_RECEIVED and result ~= receive_result.UNMATCHING_RELIABLE_RECEIVED then
		print "Receiving data..."
			while input_bs:GetNumberOfUnreadBits() >= 8 do
				local msg = protocol.read_msg(input_bs)
				
				-- server might want to copy the subject client
				msg.subject = msgs[i].subject
				
				-- if the message type has a data layout of variable size, 
				-- or it is a message altering existence of some entities,
				-- right away dispatch it to the interested party and don't post it to the entity system
				if msg.info.read_immediately ~= nil and msg.info.read_immediately == true then
					msg.input_bs = input_bs
					self.variable_message_callback(msg)
				else
					self.owner_entity_system:post_table(msg.info.name, msg)
				end
			end
		end
		
		--if result == receive_result.RELIABLE_RECEIVED then
			transmission_log:write(input_bs.read_report)
			print(input_bs.read_report)
		--end
	end
end