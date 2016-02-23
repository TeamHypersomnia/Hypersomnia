#include "item_component.h"
#include "entity_system/entity.h"

namespace components {
	void item::mark_parent_enclosing_containers_for_unmount() {
		inventory_slot_id iterated_slot = current_slot;

		while (iterated_slot.alive()) {
			auto* parent_maybe_item = iterated_slot.container_entity->find<components::item>();

			if (parent_maybe_item) {
				if (!iterated_slot->is_attachment_slot) {
					if (parent_maybe_item->is_mounted())
						parent_maybe_item->request_unmount();
				}

				iterated_slot = parent_maybe_item->current_slot;
			}
			else
				return;
		}
	}

	bool item::are_parents_last_in_lifo_slots() {
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
}