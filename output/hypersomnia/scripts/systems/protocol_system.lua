protocol_system = inherits_from (processing_system)

function protocol_system:constructor(variable_message_callback, custom_header_callback)
	self.variable_message_callback = variable_message_callback
	self.custom_header_callback = custom_header_callback
	processing_system.constructor(self)
end

function protocol_system:handle_incoming_commands()
	-- pre-clear the buffer
	local msgs = self.owner_entity_system.messages["network_message"]
	
	for i=1, #msgs do
		local input_bs = msgs[i].data:get_bitstream()
	
		local how_many_to_skip = msgs[i].channel:recv(input_bs)
		local commands_skipped = 0
		
		if how_many_to_skip ~= -1 then
			self.custom_header_callback(input_bs)
			--print "Receiving data..."
			while input_bs:GetNumberOfUnreadBits() >= 8 do
				local msg = protocol.read_msg(input_bs)
				
				-- server might want to copy the subject client
				msg.subject = msgs[i].subject
				
				msg.should_skip = commands_skipped < how_many_to_skip
				commands_skipped = commands_skipped + 1
				
				-- if the message type has a data layout of variable size, 
				-- or it is a message altering existence of some entities,
				-- right away dispatch it to the interested party and don't post it to the entity system
				if msg.info.read_immediately ~= nil and msg.info.read_immediately == true then
					msg.input_bs = input_bs
					self.variable_message_callback(msg)
				elseif not msg.should_skip then
					self.owner_entity_system:post_table(msg.info.name, msg)
				end
			end
			
		end
		
		transmission_log:write("How many to skip... " .. how_many_to_skip .. "\n")
		transmission_log:write(input_bs.read_report)
		--print("Result: " .. result .. "\n" .. input_bs.read_report)
			
		--if result == receive_result.RELIABLE_RECEIVED then
		--end
	end
end