sync_modules.movement = inherits_from ()

function sync_modules.movement:constructor()
	self.position = b2Vec2()
	self.velocity = b2Vec2()
	
	self.posvel_sig = {
		"b2Vec2", "position",
		"b2Vec2", "velocity"
	}
 end

function sync_modules.movement:read_stream(input)
	protocol.read_sig(self.posvel_sig, self, input)
end

function sync_modules.movement:read_state(input)
	self.property = input:ReadUint ()
	print ("Reading.." .. self.property)
end

-- optional
function sync_modules.movement:update_game_object(object)
	
end

-- SERVER CODE
function sync_modules.movement:write_stream(object, output)
	self.position = object.cpp_entity.physics.body:GetPosition()
	self.velocity = object.cpp_entity.physics.body:GetLinearVelocity()
	protocol.write_sig(self.posvel_sig, self, output)
end

function sync_modules.movement:write_state(output)
	--if self.property ~= nil then
	output:WriteUint (self.property)
	print ("Writing.." .. self.property)
	--end
end
