components.wield = inherits_from()

components.wield.keys = create_enum {
	"PRIMARY_WEAPON",
	"INVENTORY"
} 

function components.wield:constructor(init_table)
	self.wielded_items = {}
end