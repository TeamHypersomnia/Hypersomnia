protocol.replication_tables.register("label", {
	init_only_fields = {
		"WString", "label_str", function (object) return object.label.label_str end
	},
	
	optional_updaters = {
		label_str = function(object, new_value, replica_module)
			object.label.text = {
				{
					wstr = new_value,
					color = rgba(255, 255, 255, 255)
				}
			}
		end
	}
})
