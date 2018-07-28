#pragma once
#include "game/detail/inventory/inventory_slot_id.h"

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
		// END GEN INTROSPECTOR

#if TODO_MOUNTING
		enum mounting_state : unsigned char {
			MOUNTED,
			UNMOUNTED,
		};

		inventory_slot_id target_slot_after_unmount;

		mounting_state current_mounting = UNMOUNTED;
		mounting_state intended_mounting = UNMOUNTED;

		void set_mounted();
		void request_mount();
		void cancel_montage();
		void request_unmount();
		void request_unmount(inventory_slot_id target_slot_after_unmount);
		void reset_mounting_timer();

		bool is_mounted() const;
#endif
	};
}

namespace invariants {
	struct item {
		static constexpr bool reinfer_when_tweaking = true;
		// GEN INTROSPECTOR struct invariants::item
		bool stackable = false;
		pad_bytes<3> pad;

		item_holding_stance holding_stance = item_holding_stance::PISTOL_LIKE;

		unsigned space_occupied_per_charge = 1;

		unsigned dual_wield_accuracy_loss_percentage = 50;
		unsigned dual_wield_accuracy_loss_multiplier = 1;

		sound_effect_input wield_sound;
		sound_effect_input wear_sound;

		item_category_flagset categories_for_slot_compatibility = { item_category::GENERAL };
		money_type standard_price = 1000;
		// END GEN INTROSPECTOR
	};
}