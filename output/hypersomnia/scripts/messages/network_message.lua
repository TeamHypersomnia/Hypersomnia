network_message = inherits_from ()

function network_message:constructor(init)
	self.name = "network_message"
	self.data = init.data
end