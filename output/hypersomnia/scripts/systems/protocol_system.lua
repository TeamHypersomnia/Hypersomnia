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
		local network_msg = msgs[i]
		local input_bs = network_msg.data:get_bitstream()
	
		local how_many_to_skip = 0
		local reliable_commands_num = 0
		local commands_read = 0
		
		if not network_msg.is_reliable_transmission and network_msg.channel then
			local channel = network_msg.channel
			
			how_many_to_skip, reliable_commands_num = channel:recv(input_bs)
		else
			input_bs:IgnoreBytes(1)
		end
		
		if how_many_to_skip ~= -1 then
			--global_logfile:write ("\nSkipping " .. how_many_to_skip)
			--global_logfile:write  ("\nreliable_commands_num " .. reliable_commands_num)
			self.custom_header_callback(input_bs)
			--print "Receiving data..."
			while input_bs:GetNumberOfUnreadBits() >= 8 do
				local msg = protocol.read_msg(input_bs)
				--global_logfile:write  ("\n" .. msg.info.name)
				-- unreliable commands don't need to be packed in loops
				--global_logfile:write  ("\ncommands_read " .. commands_read)
				local is_unreliable = not msg.is_reliable_transmission and commands_read >= reliable_commands_num
				
				-- server might want to copy the subject client
				msg.subject = msgs[i].subject
				msg.guid = msgs[i].guid
				
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
					--if is_unreliable then
					--	global_logfile:write  "\nunreliable"
					--else
					--	global_logfile:write  "\nreliable"
					--end
					
					if is_unreliable or not self.pack_loops then
						--global_logfile:write  ("\nposting " .. msg.info.name)
						self.owner_entity_system:post_table(msg.info.name, msg)
					else
						if msg.info.name == "LOOP_SEPARATOR" then
							self.pending_loops[#self.pending_loops + 1] = new_loop
							new_loop = {}
							--global_logfile:write  "\nadded new loop..."
						else
							--global_logfile:write  ("\nadded " .. msg.info.name)
							new_loop[#new_loop + 1] = msg
						end
					end
				end
				
				commands_read = commands_read + 1
			end
		
			transmission_log:write("How many to skip... " .. how_many_to_skip .. "\n")
			transmission_log:write(input_bs.read_report)	
		end
		
		--print("Result: " .. how_many_to_skip .. "\n" .. input_bs.read_report)
			
		--if result == receive_result.RELIABLE_RECEIVED then
		--end
	end

end

function protocol_system:unpack_oldest_loop()
--print "unpacking...."
	if #self.pending_loops > 0 then
		local oldest_loop = self.pending_loops[1]
		
		print "LOOP!"
		for i=1, #oldest_loop do
			print (oldest_loop[i].info.name)
			self.owner_entity_system:post_table(oldest_loop[i].info.name, oldest_loop[i])
		end
		
		table.remove(self.pending_loops, 1)
	end
end

function protocol_system:has_pending_loops()
	return next(self.pending_loops) ~= nil
end
