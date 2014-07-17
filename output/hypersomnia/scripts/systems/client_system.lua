client_system = inherits_from (processing_system)

function client_system:constructor(network) 
	self.network = network
		
	-- setup reliable channel
	self.net_channel = reliable_channel_wrapper:create()
	
	self.my_sync_id = nil
	
	self.substep_unreliable = BitStream()
	
	set_rate(self, "cmd_rate", 20)
	self.cmd_requested = false
	
	self.global_time = timer()
	
	processing_system.constructor(self)
end

function client_system:get_required_components()
	return { "client" }
end


function client_system:substep()
	self.substep_unreliable:Reset()
end

function client_system:send_all_data()
	-- write all pending reliable and unreliable data
	
	
	if self.server_guid ~= nil then --and self:cmd_rate_ready() then
		--print(self.network:get_last_ping(self.server_guid))
		--self.cmd_requested = false
		--self.cmd_rate_timer:reset()
		
		self.net_channel:post_unreliable_bs(self.substep_unreliable)
		
		if self.net_channel:has_something_to_send() then
			--print "Sending data..."
			--print(self.global_time:get_milliseconds())
			local output_bs = self.net_channel:send()
			
			transmission_log:write("\nSending time: " .. self.global_time:get_milliseconds() .. "\n\n" .. auto_string_indent(output_bs.content) .. "\n\n")
			self.network:send(output_bs, send_priority.IMMEDIATE_PRIORITY, send_reliability.UNRELIABLE, 0, self.server_guid, false)
		end
	end
end