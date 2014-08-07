protocol.replication_tables.register("crosshair", {
	properties = {
		"Vec2", "position", function (object) return vec2(object.orientation.crosshair_position) end
	}
})