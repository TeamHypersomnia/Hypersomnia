#include "relational_cache.h"
#include "game/components/fixtures_component.h"
#include "game/components/motor_joint_component.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "augs/templates/enum_introspect.h"

void relational_cache::reserve_caches_for_entities(const size_t n) {
	for_each_tracker(
		[n](auto& tracker){
			tracker.reserve(n);
		}
	);
}

void relational_cache::handle_deletion_of_potential_parent(const entity_id h) {
	for_each_tracker(
		[h](auto& tracker){
			using T = std::decay_t<decltype(tracker)>;

			if constexpr(std::is_same_v<typename T::parent_id_type, inventory_slot_id>) {
				augs::for_each_enum([&](const slot_function s){
					tracker.handle_deletion_of_parent({s, h});
				});
			}
			else {
				tracker.handle_deletion_of_parent(h);
			}
		}
	);
}

void relational_cache::infer_cache_for(const const_entity_handle h) {
	if (const auto item = h.find<components::item>();
		item != nullptr
	) {
		/*
			WHEN ORDER OF ITEMS IN THE CONTAINER BECOMES RELEVANT,
			This procedure should be fixed or otherwise the reinference might break the order of items!
		*/

		ensure(!items_of_slots.is_parent_set(h));

		const auto current_slot = item->get_current_slot();

		/* Contrary to other relations, here having a parent is optional */

		if (current_slot.is_set()) {
			items_of_slots.set_parent(h, current_slot);
		}
	}

	if (const auto fixtures = h.find<components::fixtures>();
		fixtures != nullptr && fixtures.is_activated()
	) {
		ensure(!fixtures_of_bodies.is_parent_set(h));
		
		const auto owner_body = fixtures.get_owner_body();
		fixtures_of_bodies.set_parent(h, owner_body);
	}

	if (const auto motor_joint = h.find<components::motor_joint>();
		motor_joint != nullptr && motor_joint.is_activated()
	) {
		static_assert(std::is_same_v<std::array<unversioned_entity_id, 2>, decltype(joints_of_bodies)::parent_array_type>, "Make it work for more joints plx");
		
		ensure(!joints_of_bodies.is_parent_set(h, 0));
		ensure(!joints_of_bodies.is_parent_set(h, 1));

		const auto bodies = motor_joint.get_target_bodies();

		const auto& cosmos = h.get_cosmos();

		ensure(cosmos[bodies[0]].alive());
		ensure(cosmos[bodies[1]].alive());

		joints_of_bodies.set_parents(
			h, 
			std::array<unversioned_entity_id, 2> { 
				bodies[0], 
				bodies[1] 
			} 
		);
	}
}

void relational_cache::destroy_cache_of(const const_entity_handle h) {
	for_each_tracker(
		[h](auto& tracker){
			tracker.unset_parents_of(h);
		}
	);
}