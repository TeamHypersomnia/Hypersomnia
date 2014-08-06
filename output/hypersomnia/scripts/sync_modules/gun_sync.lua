protocol.replication_tables.gun_init_info = create_replication_table ({}, {
	GENERIC_UPDATER = function(object, field_name, new_value, replica_module)
		object.weapon[field_name] = new_value
		print (field_name, new_value)
	end
},
{
	"Ushort", "current_rounds", function (object) return object.weapon.current_rounds end
}
)