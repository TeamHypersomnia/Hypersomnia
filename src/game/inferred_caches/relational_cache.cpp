#include "relational_cache.h"
#include "game/components/fixtures_component.h"
#include "game/components/motor_joint_component.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "augs/templates/enum_introspect.h"

void relational_cache::infer_cache_for(const const_entity_handle h) {
	h.dispatch_on_having<components::item>([this](const auto handle) {
		/*
			WHEN ORDER OF ITEMS IN THE CONTAINER BECOMES RELEVANT,
			This procedure should be fixed or otherwise the reinference might break the order of items!
		*/

		const auto& item = handle.template get<components::item>();
		const auto current_slot = item->get_current_slot();

		/* Contrary to other relations, here having a parent is optional */

		if (current_slot.is_set()) {
			items_of_slots.assign_parenthood(handle, current_slot);
		}

		/*
			The physics system tracks the joints of bodies and fixtures of bodies for us.
		*/
	});
}

void relational_cache::destroy_cache_of(const const_entity_handle h) {
	h.dispatch_on_having<components::item>([this](const auto handle) {
		const auto& item = handle.template get<components::item>();
		const auto current_slot = item->get_current_slot();

		if (current_slot.is_set()) {
			items_of_slots.unset_parenthood(handle, current_slot);
		}
	});
}