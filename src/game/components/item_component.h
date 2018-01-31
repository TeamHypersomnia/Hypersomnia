#pragma once
#include <optional>

#include "game/components/transform_component.h"

#include "game/messages/queue_destruction.h"
#include "game/messages/item_picked_up_message.h"
#include "game/messages/interpolation_correction_request.h"

#include "game/detail/inventory/inventory_slot_id.h"

#include "game/transcendental/entity_handle_declaration.h"
#include "game/detail/inventory/inventory_slot_handle_declaration.h"
#include "game/detail/inventory/item_slot_transfer_request_declaration.h"

#include "game/enums/item_category.h"

#include "game/detail/view_input/sound_effect_input.h"

struct perform_transfer_result {
	std::optional<messages::queue_destruction> destructed;
	std::vector<messages::interpolation_correction_request> interpolation_corrected;
	std::optional<messages::item_picked_up_message> picked;

	struct drop {
		sound_effect_input sound_input;
		sound_effect_start_input sound_start;

		components::transform sound_transform;
		entity_id sound_subject;
	};

	std::optional<drop> dropped;

	void notify(logic_step) const;
};

namespace augs {
	struct introspection_access;
}

class cosmic;
class cosmos;

namespace components {
	struct item {
	private:
		friend augs::introspection_access;
		friend cosmic;

		// GEN INTROSPECTOR struct components::item
		int charges = 1;
		inventory_slot_id current_slot;
		// END GEN INTROSPECTOR

		void detail_unset_current_slot(entity_handle);

	public:
		auto get_current_slot() const {
			return current_slot;
		}

		auto get_charges() const {
			return charges;
		}

		bool try_set_charges(const int new_charges) {
			if (current_slot.is_set()) {
				return false;
			}

			charges = new_charges;
			return true;
		};

		perform_transfer_result perform_transfer(
			const item_slot_transfer_request r, 
			cosmos& cosmos
		); 

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
		using implied_component = components::item;

		// GEN INTROSPECTOR struct invariants::item
		bool stackable = false;
		pad_bytes<3> pad;

		unsigned space_occupied_per_charge = 1;

		unsigned dual_wield_accuracy_loss_percentage = 50;
		unsigned dual_wield_accuracy_loss_multiplier = 1;

		item_category_flagset categories_for_slot_compatibility = { item_category::GENERAL };
		// END GEN INTROSPECTOR
	};
}