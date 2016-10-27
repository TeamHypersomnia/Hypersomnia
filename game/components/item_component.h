#pragma once
#include <array>
#include "container_component.h"
#include "transform_component.h"
#include "game/detail/inventory_slot_id.h"
#include "game/detail/gui/item_button.h"
#include "game/transcendental/entity_handle_declaration.h"

namespace components {
	struct item {
		enum mounting_state {
			MOUNTED,
			UNMOUNTED,
		};

		item_button button;

		mounting_state current_mounting = UNMOUNTED;
		mounting_state intended_mounting = UNMOUNTED;

		unsigned categories_for_slot_compatibility = 0;

		int charges = 1;
		unsigned space_occupied_per_charge = 1;
		int stackable = false;

		unsigned dual_wield_accuracy_loss_percentage = 50;
		unsigned dual_wield_accuracy_loss_multiplier = 1;

		std::array<components::transform, 4> attachment_offsets_per_sticking_mode;

		inventory_slot_id current_slot;
		inventory_slot_id target_slot_after_unmount;

		float montage_time_ms = 1000;
		float montage_time_left_ms = 0.f;

		template<class F>
		void for_each_held_id(F f) {
			f(current_slot.container_entity);
			f(target_slot_after_unmount.container_entity);
		}

		template<class F>
		void for_each_held_id(F f) const {
			f(current_slot.container_entity);
			f(target_slot_after_unmount.container_entity);
		}

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(current_mounting),
				CEREAL_NVP(intended_mounting),

				CEREAL_NVP(categories_for_slot_compatibility),

				CEREAL_NVP(charges),
				CEREAL_NVP(space_occupied_per_charge),
				CEREAL_NVP(stackable),

				CEREAL_NVP(dual_wield_accuracy_loss_percentage),
				CEREAL_NVP(dual_wield_accuracy_loss_multiplier),

				CEREAL_NVP(attachment_offsets_per_sticking_mode),

				CEREAL_NVP(current_slot),
				CEREAL_NVP(target_slot_after_unmount),

				CEREAL_NVP(montage_time_ms),
				CEREAL_NVP(montage_time_left_ms)
			);
		}

		void set_mounted();
		void request_mount();
		void cancel_montage();
		void request_unmount();
		void request_unmount(inventory_slot_id target_slot_after_unmount);
		void reset_mounting_timer();

		void mark_parent_enclosing_containers_for_unmount();

		unsigned get_space_occupied() const;
		bool is_mounted() const;
		bool are_parents_last_in_lifo_slots() const;
		static bool can_merge_entities(const_entity_handle e1, const_entity_handle e2);
	};
}