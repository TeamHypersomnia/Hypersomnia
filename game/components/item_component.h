#pragma once
#include "container_component.h"
#include "transform_component.h"
#include "../detail/inventory_slot_id.h"

namespace components {
	struct item {
		enum mounting_state {
			MOUNTED,
			UNMOUNTED,
		};

		mounting_state current_mounting = UNMOUNTED;
		mounting_state intended_mounting = UNMOUNTED;

		unsigned categories_for_slot_compatibility = 0;

		int charges = 1;
		unsigned space_occupied_per_charge = 1;
		bool stackable = false;

		unsigned dual_wield_accuracy_loss_percentage = 50;
		unsigned dual_wield_accuracy_loss_multiplier = 1;

		std::array<components::transform, 4> attachment_offsets_per_sticking_mode;

		inventory_slot_id current_slot;
		inventory_slot_id target_slot_after_unmount;

		float montage_time_ms = 1000;
		float montage_time_left_ms = 0.f;

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
		static bool can_merge_entities(const augs::entity_id& e1, const augs::entity_id& e2);
	};
}