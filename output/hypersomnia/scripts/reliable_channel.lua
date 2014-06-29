reliable_channel_wrapper = inherits_from ()

function reliable_channel_wrapper:constructor() 
	self.channel = reliable_channel()
	
	self.channel:enable_starting_byte(protocol.GAME_TRANSMISSION)
	
	self.sender = self.channel.sender
	self.receiver = self.channel.receiver
	
	self.unreliable_buf = BitStream()
	self.sender.unreliable_buf = self.unreliable_buf
end


function reliable_channel_wrapper:post_bitstream(output_bs)
	self:post( { output_bitstream = output_bs } )
end

function reliable_channel_wrapper:post(message)
	if message["should_invalidate_message"] ~= nil then
		local msgs = self.sender.reliable_buf
		for i=0, self.sender.reliable_buf:size()-1 do
			msgs[i].flag_for_deletion = message:should_invalidate_message(msgs[i].script)
		end
	end
	
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
	return self.channel:recv(input_bs)
end

function reliable_channel_wrapper:send()
	local final_bs = BitStream()
	self.channel:send(final_bs)
	return final_bs
end