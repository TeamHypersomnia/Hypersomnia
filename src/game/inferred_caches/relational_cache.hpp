#pragma once
#include "game/inferred_caches/relational_cache.h"

template <class E>
void relational_cache::specific_infer_cache_for(const E& typed_handle) {
	/*
		WHEN ORDER OF ITEMS IN THE CONTAINER BECOMES RELEVANT,
		This procedure should be fixed or otherwise the reinference might break the order of items!
	*/

	const auto& item = typed_handle.template get<components::item>();
	const auto current_slot = item->get_current_slot();

	/* Contrary to other relations, here having a parent is optional */

	if (current_slot.is_set()) {
		items_of_slots.assign_parenthood(typed_handle, current_slot);
	}

	/*
		The physics system tracks the joints of bodies and fixtures of bodies for us.
	*/
}
