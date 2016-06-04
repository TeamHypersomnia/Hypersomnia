#include "item_component.h"
#include "entity_system/entity.h"

namespace components {
	void item::mark_parent_enclosing_containers_for_unmount() {
		inventory_slot_id iterated_slot = current_slot;

		while (iterated_slot.alive()) {
			auto* parent_maybe_item = iterated_slot.container_entity->find<components::item>();

			if (parent_maybe_item) {
				if (!iterated_slot->is_physical_attachment_slot) {
					if (parent_maybe_item->is_mounted())
						parent_maybe_item->request_unmount();
				}

				iterated_slot = parent_maybe_item->current_slot;
			}
			else
				return;
		}
	}

	bool item::are_parents_last_in_lifo_slots() const {
		inventory_slot_id iterated_slot = current_slot;

		while (iterated_slot.alive()) {
			if (iterated_slot->only_last_inserted_is_movable && (*iterated_slot->items_inside.rbegin())->find<components::item>() != this)
				return false;
			else {
				auto* maybe_item = iterated_slot.container_entity->find<components::item>();

				if (maybe_item)
					iterated_slot = maybe_item->current_slot;
				else
					return true;
			}
		}

		return true;
	}

	bool item::can_merge_entities(const augs::entity_id& e1, const augs::entity_id& e2) {
		auto* pa = e1->find<item>();
		auto* pb = e2->find<item>();
		if (!pa && !pb) return true;
		if (!(pa && pb)) return false;

		auto& a = *pa;
		auto& b = *pb;

		return
			a.stackable && b.stackable &&
			std::make_tuple(a.categories_for_slot_compatibility, a.space_occupied_per_charge, a.dual_wield_accuracy_loss_percentage, a.dual_wield_accuracy_loss_multiplier, 
				a.attachment_offsets_per_sticking_mode, a.montage_time_ms, a.current_mounting) == 
			std::make_tuple(b.categories_for_slot_compatibility, b.space_occupied_per_charge, b.dual_wield_accuracy_loss_percentage, b.dual_wield_accuracy_loss_multiplier,
				b.attachment_offsets_per_sticking_mode, b.montage_time_ms, b.current_mounting);
	}

	void item::reset_mounting_timer() {
		montage_time_left_ms = montage_time_ms * current_slot->montage_time_multiplier;
	}

	void item::cancel_montage() {
		reset_mounting_timer();
		intended_mounting = current_mounting;
		target_slot_after_unmount.unset();
	}

	bool item::is_mounted() const {
		return current_mounting == MOUNTED;
	}

	void item::request_unmount() {
		current_mounting = UNMOUNTED;
		target_slot_after_unmount = current_slot;
	}

	void item::request_unmount(inventory_slot_id target_slot_after_unmount) {
		request_unmount();
		this->target_slot_after_unmount = target_slot_after_unmount;
	}

	void item::request_mount() {
		reset_mounting_timer();
		current_mounting = MOUNTED;
	}

	void item::set_mounted() {
		current_mounting = MOUNTED;
		intended_mounting = MOUNTED;
	}

	unsigned item::get_space_occupied() const {
		return charges * space_occupied_per_charge;
	}
}