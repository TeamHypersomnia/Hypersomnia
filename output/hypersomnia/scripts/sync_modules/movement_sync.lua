sync_modules.movement = inherits_from ()

function sync_modules.movement:constructor() end

function sync_modules.movement:read_stream(object, input)
	local body = object.cpp_entity.physics.body
	
	input:name_property("position")
	local position = input:Readb2Vec2()
	input:name_property("velocity")
	local velocity = input:Readb2Vec2()
	
	
	debuglb2(rgba(255, 0, 0, 255), position, b2Vec2(position.x+velocity.x, position.y+velocity.y))
	--if my_sync ~= object.synchronization.id then 
		--body:SetTransform(position, 0)
		--body:SetLinearVelocity(velocity)
	--end 
end


function sync_modules.movement:read_state(input)
	print "reading_state"
end

function sync_modules.movement:update_game_object(object)
	
end


-- SERVER CODE

function sync_modules.movement:write_stream(object, output)
	local body = object.cpp_entity.physics.body
	
	output:name_property("position")
	output:Writeb2Vec2(body:GetPosition())
	output:name_property("velocity")
	output:Writeb2Vec2(body:GetLinearVelocity())
end

function sync_modules.movement:write_state(object, output)

end
