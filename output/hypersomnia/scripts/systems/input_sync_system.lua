input_sync_system = inherits_from (processing_system)

function input_sync_system:constructor(subject_world) 
	self.network = network
	
	-- cpp world_instance to get all intent messages
	self.subject_world = subject_world
	
	processing_system.constructor(self)
end

function input_sync_system:get_required_components()
	return { "input_sync" }
end

function input_sync_system:update()
	local msgs = self.subject_world:get_messages_filter_components("intent_message", { "input_sync" } )
				
	for i=1, #msgs do
		local msg = msgs[i]

		local subject = msg.subject.script
		
		if msg.intent == intent_message.MOVE_FORWARD or
		   msg.intent == intent_message.MOVE_BACKWARD or
		   msg.intent == intent_message.MOVE_LEFT or
		   msg.intent == intent_message.MOVE_RIGHT
		then
		
			local desired_command;
			
			if msg.state_flag then 
				desired_command = "+"
			else 
				desired_command = "-" 
			end
			
			local output_bs = BitStream()
			WriteByte(output_bs, protocol.message.COMMAND)
			WriteByte(output_bs, protocol.name_to_command[desired_command .. protocol.intent_to_name[msg.intent]])
			
			self.owner_entity_system.all_systems["client"].reliable_sender:post_bitstream(output_bs)
		end
		
	end
end