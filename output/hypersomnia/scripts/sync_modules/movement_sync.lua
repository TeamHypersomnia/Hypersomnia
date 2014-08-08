protocol.replication_tables.register("movement", {
	properties = {
		"b2Vec2", "position", function (object) if object.cpp_entity.physics ~= nil then return b2Vec2(object.cpp_entity.physics.body:GetPosition()) else return b2Vec2(0, 0) end end,
		"b2Vec2", "velocity", function (object) if object.cpp_entity.physics ~= nil then return b2Vec2(object.cpp_entity.physics.body:GetLinearVelocity()) else return b2Vec2(0, 0) end end
	}
})

protocol.replication_tables.register("movement_rotated", {
	properties = {
		"b2Vec2", "position", function (object) if object.cpp_entity.physics ~= nil then return b2Vec2(object.cpp_entity.physics.body:GetPosition()) else return b2Vec2(0, 0) end end,
		"b2Vec2", "velocity", function (object) if object.cpp_entity.physics ~= nil then return b2Vec2(object.cpp_entity.physics.body:GetLinearVelocity()) else return b2Vec2(0, 0) end end,
		"Float", "angle",     function (object) if object.cpp_entity.physics ~= nil then return object.cpp_entity.physics.body:GetAngle() else return 0 end end,
		"Float", "angular_velocity", function (object) if object.cpp_entity.physics ~= nil then return object.cpp_entity.physics.body:GetAngularVelocity() else return 0 end end
	}
})