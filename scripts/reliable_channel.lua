reliable_sender_class = inherits_from ()

function reliable_sender_class:constructor() 
	self.channel = reliable_sender()
end

function reliable_sender_class:post(message)
	if message["should_remove_message"] ~= nil then
		local msgs = self.channel.reliable_buf
		for i=1, #msgs do
			msgs[i].flag_for_deletion = message:should_remove_message(msgs[i].script)
		end
	end
	
	local new_msg = net_channel_message()
	new_msg.script = message
	new_msg.output_bitstream = message.output_bitstream
	
	self.channel:post_message(new_msg)
end