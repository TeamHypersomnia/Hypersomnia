protocol.replication_tables.movement = create_replication_table {
	"b2Vec2", "position", function (object) if object.cpp_entity.physics ~= nil then return b2Vec2(object.cpp_entity.physics.body:GetPosition()) else return b2Vec2(0, 0) end end,
	"b2Vec2", "velocity", function (object) if object.cpp_entity.physics ~= nil then return b2Vec2(object.cpp_entity.physics.body:GetLinearVelocity()) else return b2Vec2(0, 0) end end
}