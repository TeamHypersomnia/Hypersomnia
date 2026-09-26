#pragma once
#include "game/detail/inventory/inventory_slot_id.h"
#include "game/detail/inventory/inventory_slot_types.h"

#include "augs/pad_bytes.h"
#include "augs/misc/timing/stepped_timing.h"
#include "game/enums/item_category.h"
#include "game/enums/item_holding_stance.h"

#include "game/detail/economy/money_type.h"
#include "game/detail/view_input/sound_effect_input.h"

/*
	Items thrown or dropped explicitly hit the floor a few times before they come to rest - see item_falling.h.
	when_landed is when they came to rest after the last hit.
*/

struct item_fall_state {
	// GEN INTROSPECTOR struct item_fall_state
	augs::stepped_timestamp when_landed;
	augs::stepped_timestamp when_started_falling;
	augs::stepped_timestamp when_hop_started;
	real32 hop_duration_secs = 0.f;
	real32 hop_height = 1.f;
	uint8_t floor_hits_left = 0;
	uint8_t floor_hits_done = 0;
	bool thrown_up = false;
	bool thrown_melee = false;
	// END GEN INTROSPECTOR
};

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
	struct item {
		static constexpr bool reinfer_when_tweaking = true;
		// GEN INTROSPECTOR struct invariants::item
		inventory_space_type space_occupied_per_charge = 1;
		item_category_flagset categories_for_slot_compatibility = { item_category::GENERAL };
		item_holding_stance holding_stance = item_holding_stance::BARE_LIKE;

		bool stackable = false;
		bool draw_under_hands_in_akimbo = false;
		pad_bytes<2> pad;

		sound_effect_input wield_sound;
		sound_effect_input wear_sound;

		money_type standard_price = 0;

		bool flip_when_reloading = true;
		bool draw_mag_over_when_reloading = true;
		bool draw_over_hands = true;
		bool draw_over_hands_when_reloading = true;

		faction_type specific_to = faction_type::SPECTATOR;
		unsigned gratis_ammo_pieces_with_first = 0;

		transformi hand_anchor_offset;
		// END GEN INTROSPECTOR
	};
}