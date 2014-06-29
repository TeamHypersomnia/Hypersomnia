sync_modules.movement = inherits_from ()

function sync_modules.movement:constructor() end

function sync_modules.movement:read_stream(object, input)
	local body = object.cpp_entity.physics.body
	
	local position = input:Readb2Vec2()
	local velocity = input:Readb2Vec2()
	
	body:SetTransform(position, 0)
	body:SetLinearVelocity(velocity)
end


function sync_modules.movement:read_state(input)
	
end

function sync_modules.movement:update_game_object(object)
	
end


-- SERVER CODE

function sync_modules.movement:write_stream(object, output)
	local body = object.cpp_entity.physics.body
	
	output:Writeb2Vec2(body:GetPosition())
	output:Writeb2Vec2(body:GetLinearVelocity())
end

function sync_modules.movement:write_state(object, output)

end
