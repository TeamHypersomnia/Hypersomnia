protocol.replication_tables.health = create_replication_table ({}, {
	GENERIC_UPDATER = function(object, field_name, new_value, replica_module)
		object.health[field_name] = new_value
	end
},
{
	"Float", "hp", function (object) return object.health.hp end
}
)