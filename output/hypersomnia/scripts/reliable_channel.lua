reliable_channel = inherits_from ()

function reliable_channel:constructor() 
	self.sender = reliable_sender()
	self.receiver = reliable_receiver()
	
	self.unreliable_buf = BitStream()
	self.sender.unreliable_buf = self.unreliable_buf
end


function reliable_channel:post_bitstream(output_bs)
	self:post( { output_bitstream = output_bs } )
end

function reliable_channel:post(message)
	if message["should_invalidate_message"] ~= nil then
		local msgs = self.sender.reliable_buf
		for i=1, #msgs do
			msgs[i].flag_for_deletion = message:should_invalidate_message(msgs[i].script)
		end
	end
	
	local new_msg = net_channel_message()
	new_msg.script = message
	new_msg.output_bitstream = message.output_bitstream
	
	self.sender:post_message(new_msg)
end

function reliable_channel:recv(input_bs)
	-- ignore protocol.GAME_TRANSMISSION
	input_bs:IgnoreBytes(1)
	
	-- for now, all incoming network messages will redundantly contain an ack sequence
	-- receive acknowledgement from remote receiver
	self.sender:read_ack(input_bs)
	
	-- read sequence from remote sender
	local recv_result = self.receiver:read_sequence(input_bs)
	-- all internals are read from the bitstream and it is free to pass to other logic
	
	-- if there are some commands to be read from the remote sender
	if recv_result == receive_result.RELIABLE_RECEIVED then
		-- we must send an acknowledgement on the next call to write, even if there is no data to send
		self.ack_requested = true
	end
	
	return recv_result
end

function reliable_channel:write_data()
	local output_bs = BitStream()
	local final_bs = BitStream()
	
	-- if there is anything new going on or we have just got a reliable command from the remote sender, 
	-- send updates over UDP/IP
	if self.sender:write_data(output_bs) or self.ack_requested then
		-- write a dumb byte to differentiate from raknet's network messages 
		final_bs:WriteByte(protocol.GAME_TRANSMISSION)
	
		-- redundantly write an ack for the the remote sender
		self.receiver:write_ack(final_bs)
		self.ack_requested = false
		
		final_bs:WriteBitstream(output_bs)	
	end
	
	return final_bs
end