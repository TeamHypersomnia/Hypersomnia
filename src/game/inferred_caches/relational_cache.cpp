#include "relational_cache.h"
#include "game/components/fixtures_component.h"
#include "game/components/motor_joint_component.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "augs/templates/enum_introspect.h"

void relational_cache::destroy_caches_of_children_of(const entity_id h) {
	for_each_tracker(
		[h](auto& tracker){
			using T = std::decay_t<decltype(tracker)>;

			if constexpr(std::is_same_v<typename T::parent_id_type, inventory_slot_id>) {
				augs::for_each_enum([&](const slot_function s){
					tracker.destroy_caches_of_children_of({s, h});
				});
			}
			else {
				tracker.destroy_caches_of_children_of(h);
			}
		}
	);
}

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
			ensure(!items_of_slots.is_child_constructed(handle, current_slot));
			items_of_slots.set_parent(handle, current_slot);
		}

		/*
			The physics system tracks the joints of bodies and fixtures of bodies for us.
		*/
	});
}

void relational_cache::destroy_cache_of(const const_entity_handle h) {
	for_each_tracker(
		[h](auto& tracker){
			tracker.unset_parents_of(h);
		}
	);
}