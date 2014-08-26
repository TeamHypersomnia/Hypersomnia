protocol_system = inherits_from (processing_system)

function protocol_system:constructor(variable_message_callback, custom_header_callback)
	self.variable_message_callback = variable_message_callback
	self.custom_header_callback = custom_header_callback
	
	self.pending_loops = {}
	
	self.pack_loops = false
	
	processing_system.constructor(self)
end

function protocol_system:handle_incoming_commands()
	-- pre-clear the buffer
	local msgs = self.owner_entity_system.messages["network_message"]
	local new_loop = {}
	
	for i=1, #msgs do
		local msg = msgs[i]
		local input_bs = msg.data:get_bitstream()
	
		local how_many_to_skip = 0
		local reliable_commands_num = 0
		local commands_read = 0
		
		if not msg.is_reliable_transmission then 
			how_many_to_skip = msgs[i].channel:recv(input_bs)
			reliable_commands_num = msg.channel.receiver.last_message - msg.channel.receiver.first_message
		else
			input_bs:IgnoreBytes(1)
		end
		
		if how_many_to_skip ~= -1 then
			self.custom_header_callback(input_bs)
			--print "Receiving data..."
			while input_bs:GetNumberOfUnreadBits() >= 8 do
				local msg = protocol.read_msg(input_bs)
				-- unreliable commands don't need to be packed in loops
				local is_unreliable = not msg.is_reliable_transmission and commands_read >= reliable_commands_num
				
				-- server might want to copy the subject client
				msg.subject = msgs[i].subject
				
				local should_skip = not msg.is_reliable_transmission and commands_read < how_many_to_skip
				
				if msg.info.variable_size ~= nil and msg.info.variable_size == true then
					local num_bits = self.variable_message_callback(msg)
					
					if not should_skip then
						msg.input_bs = BitStream()
						msg.input_bs:WriteBits(input_bs, num_bits)
					else
						input_bs:IgnoreBits(num_bits)
					end
				end
				
				if not should_skip then
					if is_unreliable or not self.pack_loops then
						self.owner_entity_system:post_table(msg.info.name, msg)
					else
						if msg.info.name == "LOOP_SEPARATOR" then
							self.pending_loops[#self.pending_loops + 1] = new_loop
							new_loop = {}
						else
							new_loop[#new_loop + 1] = msg
						end
					end
				end
				
				commands_read = commands_read + 1
			end
			
		end
		
		transmission_log:write("How many to skip... " .. how_many_to_skip .. "\n")
		transmission_log:write(input_bs.read_report)
		--print("Result: " .. how_many_to_skip .. "\n" .. input_bs.read_report)
			
		--if result == receive_result.RELIABLE_RECEIVED then
		--end
	end

end

function protocol_system:unpack_oldest_loop()
	if #self.pending_loops > 0 then
		local oldest_loop = self.pending_loops[1]
		
		for i=1, #oldest_loop do
			self.owner_entity_system:post_table(oldest_loop[i].info.name, oldest_loop[i])
		end
		
		table.remove(self.pending_loops, 1)
	end
end

function protocol_system:has_pending_loops()
	return next(self.pending_loops) ~= nil
end
