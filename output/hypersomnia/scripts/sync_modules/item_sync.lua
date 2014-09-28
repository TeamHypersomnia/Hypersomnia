protocol.replication_tables.register("item", {
	init_only_fields = {
		"Bool", "is_wielded", function (object) return object.item.wielder ~= nil end,
		"Ushort", "wielder_id", function (object) if object.item.wielder ~= nil then return object.item.wielder.replication.id else return 0 end end,
		"Ushort", "remote_wielding_key", function (object) if object.item.wielder ~= nil then print(object.item.wielding_key) return object.item.wielding_key else return 0 end end
	},
	
	optional_updaters = {
		GENERIC_UPDATER = function(object, field_name, new_value, replica_module)
			object.item[field_name] = new_value
		end
	}
})