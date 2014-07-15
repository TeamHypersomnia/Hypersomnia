reliable_channel_wrapper = inherits_from ()

function reliable_channel_wrapper:constructor() 
	self.channel = reliable_channel()
	
	self.channel:enable_starting_byte(protocol.GAME_TRANSMISSION)
	
	self.sender = self.channel.sender
	self.receiver = self.channel.receiver
	
	self.sender.message_indexing = true
	self.receiver.message_indexing = true
	
	-- acknowledged, cleared, appended automatically to output bs
	self.reliable_sequenced_messages = {}
	-- only acknowledged and cleared, appended manually
	self.reliable_sequenced_bitstreams = {}
	
	self.unreliable_buf = self.sender.unreliable_buf
end

function reliable_channel_wrapper:has_something_to_send()
	return next(self.reliable_sequenced_messages) ~= nil or
		   next(self.reliable_sequenced_bitstreams) ~= nil or
			self.unreliable_buf:size() > 0 or self.sender.reliable_buf:size() > 0 or self.receiver.ack_requested
end

-- only the most recent message is guaranteed to arrive
function reliable_channel_wrapper:reliable_seq_bs(channel, out_bs)
	-- treat "name" as slot identifier
	self.reliable_sequenced_bitstreams[name] = {
		-- will "probably" be issued on the next update
		sequence = self.sender.unreliable_sequence+1,
		bitstream = out_bs
	}
end

-- for manually appended messages (e.g. module data while updating streams)
function reliable_channel_wrapper:get_reliable_seq_bs(channel)
	local entry = self.reliable_sequenced_bitstreams[channel]
	
	if entry == nil then
		return BitStream()
	else
		return entry.bitstream
	end
end

function reliable_channel_wrapper:reliable_seq_msg(name, entry, retry)
	local data = protocol.write_msg(name, entry)
	local slot = { bitstream = data }
	
	if retry ~= nil and retry == true then
		-- will be issued on the next update
		slot.sequence = self.sender.unreliable_sequence+1
	end
	
	-- treat "name" as slot identifier
	self.reliable_sequenced_messages[name] = slot
end


function reliable_channel_wrapper:post_reliable_bs(output_bs)
	self:post( { output_bitstream = output_bs } )
end


function reliable_channel_wrapper:post_reliable(name, entry)
	self:post_reliable_bs(protocol.write_msg(name, entry))
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
	local result = self.channel:recv(input_bs)
	
	-- invalidate reliable sequenced slots
	for k, v in pairs(self.reliable_sequenced_messages) do
		if v.sequence ~= nil and v.sequence <= self.sender.unreliable_ack_sequence then
			self.reliable_sequenced_messages[k] = nil
		end
	end
	
	for k, v in pairs(self.reliable_sequenced_bitstreams) do
		if v.sequence ~= nil and v.sequence <= self.sender.unreliable_ack_sequence then
			self.reliable_sequenced_bitstreams[k] = nil
		end
	end
	
	return result
end

function reliable_channel_wrapper:send()
	for k, v in pairs(self.reliable_sequenced_messages) do
		if v.sequence then
			self.sender.request_ack_for_unreliable = true
		end
		
		self:post_unreliable_bs(v.bitstream)
	end
	
	for k, v in pairs(self.reliable_sequenced_bitstreams) do
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