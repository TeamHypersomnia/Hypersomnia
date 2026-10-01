#pragma once
#include <array>
#include "game/detail/inventory/inventory_slot_id.h"
#include "game/detail/inventory/inventory_slot_types.h"

#include "augs/misc/timing/stepped_timing.h"
#include "game/components/item_fall_state.h"
#include "game/enums/item_category.h"
#include "game/enums/item_holding_stance.h"

#include "game/detail/economy/money_type.h"
#include "game/detail/view_input/sound_effect_input.h"

struct item_owner_meta {
	// GEN INTROSPECTOR struct item_owner_meta
	signi_entity_id original_owner;
	// END GEN INTROSPECTOR
};

namespace components {
	struct item {
		static constexpr bool is_synchronized = true;

		// GEN INTROSPECTOR struct components::item
		int charges = 1;
		signi_inventory_slot_id current_slot;
		signi_inventory_slot_id previous_slot;
		augs::stepped_timestamp when_last_transferred;
		item_owner_meta owner_meta;
		uint32_t incoming_transfer_id = 0;
		item_fall_state fall;
		// END GEN INTROSPECTOR

		void clear_slot_info() {
			current_slot.unset();
			previous_slot.unset();
		}
	};
}

namespace invariants {
	/*
		ground_scale enlarges the item lying on the ground - see calc_ground_item_scale -
		and ground_scale_affects_body decides whether its body grows too.
		low_ammo_threshold of a magazine is the low ammo cue threshold of its gun,
		so that a magazine lying on the ground knows it without its gun - see calc_magazine_ammo_state.
	*/

	struct item {
		static constexpr bool reinfer_when_tweaking = true;
		// GEN INTROSPECTOR struct invariants::item
		inventory_space_type space_occupied_per_charge = 1;
		item_category_flagset categories_for_slot_compatibility = { item_category::GENERAL };
		item_holding_stance holding_stance = item_holding_stance::BARE_LIKE;

		bool stackable = false;
		bool draw_under_hands_in_akimbo = false;
		bool ground_scale_affects_body = true;
		pad_bytes<1> pad;

		sound_effect_input wield_sound;
		sound_effect_input wear_sound;
		floor_hit_sounds_array floor_hit_sounds;

		money_type standard_price = 0;

		bool flip_when_reloading = true;
		bool draw_mag_over_when_reloading = true;
		bool draw_over_hands = true;
		bool draw_over_hands_when_reloading = true;

		faction_type specific_to = faction_type::SPECTATOR;
		unsigned gratis_ammo_pieces_with_first = 0;

		transformi hand_anchor_offset;

		real32 ground_scale = 1.f;
		unsigned low_ammo_threshold = 0;
		// END GEN INTROSPECTOR
	};
}