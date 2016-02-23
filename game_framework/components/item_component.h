#pragma once
#include "container_component.h"
#include "transform_component.h"
#include "../shared/inventory_slot_id.h"

namespace components {
	struct item {
		enum mounting_state {
			MOUNTED,
			UNMOUNTED,
		};

		mounting_state current_mounting = UNMOUNTED;
		mounting_state intended_mounting = UNMOUNTED;

		unsigned categories_for_slot_compatibility = 0;

		unsigned charges = 1;
		float space_occupied_per_charge = 1;

		unsigned dual_wield_accuracy_loss_percentage = 50;
		unsigned dual_wield_accuracy_loss_multiplier = 1;

		vec2 attachment_offsets_per_sticking_mode[4];

		inventory_slot_id current_slot;
		inventory_slot_id target_slot_after_unmount;

		float get_space_occupied() const {
			return charges * space_occupied_per_charge;
		}

		float montage_time_ms = 1000;
		float montage_time_left_ms = 0.f;

		void reset_mounting_timer() {
			montage_time_left_ms = montage_time_ms * current_slot->montage_time_multiplier;
		}

		void cancel_montage() {
			reset_mounting_timer();
			intended_mounting = current_mounting;
			target_slot_after_unmount.unset();
		}

		bool is_mounted() {
			return current_mounting == MOUNTED;
		}

		void request_unmount() {
			current_mounting = UNMOUNTED;
			target_slot_after_unmount = current_slot;
		}

		void request_unmount(inventory_slot_id target_slot_after_unmount) {
			request_unmount();
			this->target_slot_after_unmount = target_slot_after_unmount;
		}

		void request_mount() {
			reset_mounting_timer();
			current_mounting = MOUNTED;
		}

		void set_mounted() {
			current_mounting = MOUNTED;
			intended_mounting = MOUNTED;
		}

		void mark_parent_enclosing_containers_for_unmount();

		bool are_parents_last_in_lifo_slots();
	};
}