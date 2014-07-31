protocol.replication_tables.movement = create_replication_table {
	"b2Vec2", "position", function (object) return b2Vec2(object.cpp_entity.physics.body:GetPosition()) end,
	"b2Vec2", "velocity", function (object) return b2Vec2(object.cpp_entity.physics.body:GetLinearVelocity()) end
}