#pragma once
#include "game/inferred_caches/relational_cache.h"
#include "game/cosmos/get_corresponding.h"

template <class S, class M>
void unset_parenthood(const S& slot, const M& item) {
	slot.get_container().template dispatch_on_having_all<invariants::container>(
		[&](const auto& typed_container) {
			auto& tracked_children = get_corresponding<items_of_slots_cache>(typed_container).tracked_children[slot.get_type()];
			erase_element(tracked_children, static_cast<entity_id>(item));
		}
	);
}

template <class S, class M>
void assign_parenthood(const S& slot, const M& item) {
	slot.get_container().template dispatch_on_having_all<invariants::container>(
		[&](const auto& typed_container) {
			auto& tracked_children = get_corresponding<items_of_slots_cache>(typed_container).tracked_children[slot.get_type()];

			const auto item_id = entity_id(item);
			ensure(!found_in(tracked_children, item_id));
			tracked_children.emplace_back(item_id);
		}
	);
}

template <class E>
void relational_cache::specific_infer_cache_for(const E& typed_handle) {
	/*
		WHEN ORDER OF ITEMS IN THE CONTAINER BECOMES IMPORTANT,
		This procedure should be fixed or otherwise the reinference might break the order of items!
	*/

	const auto& item = typed_handle.template get<components::item>();
	const auto slot = typed_handle.get_cosmos()[item->get_current_slot()];

	/* Contrary to other relations, here having a parent is optional */

	if (slot) {
		assign_parenthood(slot, typed_handle);
	}

	/*
		The physics system tracks the joints of bodies and fixtures of bodies for us.
	*/
}
