client_system = inherits_from (processing_system)

function client_system:constructor(network) 
	self.network = network
		
	-- setup reliable channel
	self.net_channel = reliable_channel_wrapper:create()
	
	self.my_sync_id = nil
	
	set_rate(self, "cmd_rate", 20)
	self.cmd_requested = false
	
	processing_system.constructor(self)
end

function client_system:get_required_components()
	return { "client" }
end

function client_system:update_tick()
	-- write all pending reliable and unreliable data
	local output_bs = self.net_channel:send()
	
	if self.server_guid ~= nil and output_bs:size() > 0 and (self.cmd_rate_timer:get_milliseconds() > self.cmd_rate_interval_ms or self.cmd_requested) then
		self.cmd_requested = false
		self.cmd_rate_timer:reset()
		
		print("Sending: \n\n" .. auto_string_indent(output_bs.content) .. "\n\n")
		self.network:send(output_bs, send_priority.IMMEDIATE_PRIORITY, send_reliability.UNRELIABLE, 0, self.server_guid, false)
	end
end