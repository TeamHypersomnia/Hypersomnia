protocol.replication_tables.wield = create_replication_table ({}, {
	GENERIC_UPDATER = function(object, field_name, new_value, replica_module)
		object.wield[field_name] = new_value
	end
},
{
	"Bool", "is_wielding_anything", function (object) return object.wield.wielded_item ~= nil end,
	"Ushort", "wielded_item_id", function (object) if object.wield.wielded_item ~= nil then return object.wield.wielded_item.replication.id else return 0 end end
}
)