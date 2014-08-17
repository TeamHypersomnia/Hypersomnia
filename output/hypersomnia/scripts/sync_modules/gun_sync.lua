protocol.replication_tables.register("gun_init_info", {
	init_only_fields = {
		"Ushort", "current_rounds", function (object) return object.weapon.current_rounds end
	},
	
	optional_updaters = {
		GENERIC_UPDATER = function(object, field_name, new_value, replica_module)
			object.weapon[field_name] = new_value
		end
	}
})
