reliable_channel_wrapper = inherits_from ()

function reliable_channel_wrapper:constructor() 
	self.channel = reliable_channel()
	
	self.channel:enable_starting_byte(protocol.GAME_TRANSMISSION)
	
	self.sender = self.channel.sender
	self.receiver = self.channel.receiver
	
	self.sender.message_indexing = true
	self.receiver.message_indexing = true
	
	self.last_reliable_msg_type = "NONE"
	
	-- acknowledged, cleared, appended automatically to output bs
	self.reliable_sequenced_messages = {}
	
	-- only acknowledged and cleared, appended manually
	self.reliable_sequenced_callbacks = {}
	
	self.unreliable_buf = self.sender.unreliable_buf
end

function reliable_channel_wrapper:has_something_to_send()
	return next(self.reliable_sequenced_messages) ~= nil or
			self.unreliable_buf:size() > 0 or self.sender.reliable_buf:size() > 0 or self.receiver.ack_requested
end

function reliable_channel_wrapper:next_unreliable_sequence()
	return self.sender.unreliable_sequence+1
end

function reliable_channel_wrapper:unreliable_ack()
	return self.sender.unreliable_ack_sequence
end

function reliable_channel_wrapper:reliable_seq_msg(name, entry, retry)
	local data = protocol.write_msg(name, entry)
	local slot = { bitstream = data }
	
	if retry ~= nil and retry == true then
		-- will be issued on the next update
		slot.sequence = self:next_unreliable_sequence()
	end
	
	-- treat "name" as slot identifier
	self.reliable_sequenced_messages[name] = slot
end


function reliable_channel_wrapper:post_reliable_bs(output_bs)
	self:post( { output_bitstream = output_bs } )
	self.last_reliable_msg_type = "RELIABLE_BITSTREAM"
end


function reliable_channel_wrapper:post_reliable(name, entry)
	self:post_reliable_bs(protocol.write_msg(name, entry))
	self.last_reliable_msg_type = name
end

function reliable_channel_wrapper:post_unreliable_bs(output_bs)
	self.unreliable_buf:WriteBitstream(output_bs)
end

function reliable_channel_wrapper:post_unreliable(name, entry)
	self:post_unreliable_bs(protocol.write_msg(name, entry))
end

function reliable_channel_wrapper:post(message)
	local new_msg = net_channel_message()
	new_msg.script = message
	new_msg.output_bitstream = message.output_bitstream
	
	self.sender:post_message(new_msg)
end


function reliable_channel_wrapper:enable_starting_byte(...)
	self.channel:enable_starting_byte(...)
end

function reliable_channel_wrapper:disable_starting_byte(...)
	self.channel:disable_starting_byte(...)
end

function reliable_channel_wrapper:recv(input_bs)	
	local how_many_to_skip = self.channel:recv(input_bs)
	
	-- invalidate reliable sequenced slots
	for k, v in pairs(self.reliable_sequenced_messages) do
		if v.sequence ~= nil and v.sequence <= self:unreliable_ack() then
			self.reliable_sequenced_messages[k] = nil
		end
	end
	
	local reliable_commands_num = 0
	
	if self.receiver.has_reliable then
		reliable_commands_num = self.receiver.last_message - self.receiver.first_message
	end
	
	return how_many_to_skip, reliable_commands_num
end

function reliable_channel_wrapper:send()
	for k, v in pairs(self.reliable_sequenced_messages) do
		if v.sequence then
			self.sender.request_ack_for_unreliable = true
		end
		
		self:post_unreliable_bs(v.bitstream)
	end
	
	local final_bs = BitStream()
	self.channel:send(final_bs)
	
	self.unreliable_buf:Reset()
	
	return final_bs
end