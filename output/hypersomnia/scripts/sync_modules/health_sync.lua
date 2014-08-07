protocol.replication_tables.register("health", {
	optional_updaters = {
		GENERIC_UPDATER = function(object, field_name, new_value, replica_module)
			object.health[field_name] = new_value
		end
	},
	
	init_only_fields = {
		"Float", "hp", function (object) return object.health.hp end
	}
})