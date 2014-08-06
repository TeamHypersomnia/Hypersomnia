protocol.replication_tables.item = create_replication_table ({}, {
	GENERIC_UPDATER = function(object, field_name, new_value, replica_module)
		object.item[field_name] = new_value
	end
},
{
	"Bool", "is_owned", function (object) return object.item.ownership ~= nil end,
	"Ushort", "ownership_id", function (object) if object.item.ownership ~= nil then return object.item.ownership.replication.id else return 0 end end
}
)