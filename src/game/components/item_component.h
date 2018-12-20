#pragma once
#include "game/detail/inventory/inventory_slot_id.h"
#include "game/detail/inventory/inventory_slot_types.h"

#include "augs/misc/timing/stepped_timing.h"
#include "game/enums/item_category.h"
#include "game/enums/item_holding_stance.h"

#include "game/detail/economy/money_type.h"
#include "game/detail/view_input/sound_effect_input.h"

namespace components {
	struct item {
		static constexpr bool is_synchronized = true;

		// GEN INTROSPECTOR struct components::item
		int charges = 1;
		signi_inventory_slot_id current_slot;
		augs::stepped_timestamp when_last_transferred;
		// END GEN INTROSPECTOR
	};
}

namespace invariants {
	struct item {
		static constexpr bool reinfer_when_tweaking = true;
		// GEN INTROSPECTOR struct invariants::item
		inventory_space_type space_occupied_per_charge = 1;
		item_category_flagset categories_for_slot_compatibility = { item_category::GENERAL };
		item_holding_stance holding_stance = item_holding_stance::BARE_LIKE;

		bool stackable = false;
		pad_bytes<3> pad;

		sound_effect_input wield_sound;
		sound_effect_input wear_sound;

		money_type standard_price = 0;

		bool flip_when_reloading = true;
		bool draw_mag_over_when_reloading = true;
		pad_bytes<2> pad2;

		faction_type specific_to = faction_type::SPECTATOR;
		unsigned gratis_ammo_pieces_with_first = 0;
		// END GEN INTROSPECTOR
	};
}