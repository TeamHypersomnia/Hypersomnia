server_commands = inherits_from ()

function server_commands:constructor(init)
	self.name = "server_commands"
	self.subject = init.subject
	self.bitstream = init.bitstream
end

function server_commands:get_bitstream()
	return copy_bitstream_for_reading(self.bitstream)
end