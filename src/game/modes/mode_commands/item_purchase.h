#pragma once
#include "game/enums/faction_type.h"
#include "game/organization/special_flavour_id_types.h"
#include "game/detail/spells/all_spells.h"

namespace mode_commands {
	struct item_purchase {
		// GEN INTROSPECTOR struct mode_commands::item_purchase
		item_flavour_id item;
		spell_id spell;
		// END GEN INTROSPECTOR

		bool is_set() const {
			return item.is_set() || spell.is_set();
		}

		bool operator==(const item_purchase& b) const {
			return 
				item == b.item
				&& spell == b.spell
			;
		}
	};
}
