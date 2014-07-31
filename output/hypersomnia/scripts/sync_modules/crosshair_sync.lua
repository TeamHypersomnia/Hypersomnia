protocol.replication_tables.crosshair = create_replication_table {
	"Vec2", "position", function (object) return vec2(object.orientation.crosshair_position) end
}