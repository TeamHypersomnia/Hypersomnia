protocol.replication_tables.client_info = create_replication_table ({
	"Ushort", "controlled_object_id", function (object) return vec2(object.client_info.controlled_object_id) end
}, {
	GENERIC_UPDATER = function(object, field_name, new_value, replica_module)
		object.client_info[field_name] = new_value
	end
})