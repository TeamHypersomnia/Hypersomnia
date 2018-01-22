#pragma once
#include "augs/misc/children_vector_tracker.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/detail/inventory/inventory_slot_id.h"

class relational_cache {
public:
	augs::children_vector_tracker<entity_id, inventory_slot_id, 1> items_of_slots;

	template <class F>
	void for_each_tracker(F f) {
		f(items_of_slots);
	}

	void infer_cache_for(const const_entity_handle);
	void destroy_cache_of(const const_entity_handle);

	void destroy_caches_of_children_of(const entity_id);

	void set_current_slot(
		const entity_id item, 
		const inventory_slot_id new_slot
	) {
		items_of_slots.set_parent(item, new_slot);
	}

	const auto& get_items_of_slots() const {
		return items_of_slots;
	}
};