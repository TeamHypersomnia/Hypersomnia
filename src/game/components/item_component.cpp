#include "item_component.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"

namespace components {
#if TODO
	void item::mark_parent_enclosing_containers_for_unmount() {
		inventory_slot_id iterated_slot = current_slot;
	
		while (iterated_slot.alive()) {
			auto parent_maybe_item = iterated_slot.container_entity.find<components::item>();
	
			if (parent_maybe_item) {
				if (!iterated_slot->is_physical_attachment_slot) {
					if (parent_maybe_item->is_mounted())
						parent_maybe_item->request_unmount();
				}
	
				iterated_slot = parent_maybe_item->get_current_slot();
			}
			else
				return;
		}
	}
	
	bool item::are_parents_last_in_lifo_slots() const {
		inventory_slot_id iterated_slot = current_slot;
	
		while (iterated_slot.alive()) {
	  //		if (iterated_slot->only_last_inserted_is_movable && (*iterated_slot->get_items_inside().rbegin()).find<components::item>() != this)
				return false;
			else {
				auto maybe_item = iterated_slot.container_entity.find<components::item>();
	
				if (maybe_item)
					iterated_slot = maybe_item->get_current_slot();
				else
					return true;
			}
		}
	
		return true;
	}
	
	void item::reset_mounting_timer() {
		montage_time_left_ms = montage_time_ms * current_slot->montage_time_multiplier;
	}

	void item::cancel_montage() {
		reset_mounting_timer();
		intended_mounting = current_mounting;
		target_slot_after_unmount.unset();
	}
	
	void item::request_mount() {
		reset_mounting_timer();
		current_mounting = MOUNTED;
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

	void item::set_mounted() {
		current_mounting = MOUNTED;
		intended_mounting = MOUNTED;
	}
#endif
}