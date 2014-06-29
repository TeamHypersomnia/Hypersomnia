sync_modules.movement = inherits_from ()

function sync_modules.movement:constructor() end

function sync_modules.movement:read_stream(object, input)
	local body = object.cpp_entity.physics.body
	
	local position = Readb2Vec2(input)
	local velocity = Readb2Vec2(input)
	
	body:SetTransform(position, 0)
	body:SetLinearVelocity(velocity)
end


function sync_modules.movement:read_state(input)
	
end

function sync_modules.movement:update_game_object(object)
	
end


-- SERVER CODE

function sync_modules.movement:write_stream(object, output)
	local body = self.parent_entity.physics.body
	
	Writeb2Vec2(output, body:GetPosition())
	Writeb2Vec2(output, body:GetLinearVelocity())
end

function sync_modules.movement:upload_state(object, output)

end
