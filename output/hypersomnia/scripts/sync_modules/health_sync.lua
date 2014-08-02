protocol.replication_tables.client_info = create_replication_table ({
	"Float", "hp", function (object) return object.health.hp end
}, {
	GENERIC_UPDATER = function(object, field_name, new_value, replica_module)
		object.health[field_name] = new_value
	end
})