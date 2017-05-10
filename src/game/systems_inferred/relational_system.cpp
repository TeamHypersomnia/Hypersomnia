#include "relational_system.h"
#include "game/components/fixtures_component.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

void relational_system::reserve_caches_for_entities(const size_t n) {
	for_each_tracker(
		[n](auto& tracker){
			tracker.reserve(n);
		}
	);
}

void relational_system::handle_deletion_of_potential_parent(const entity_id h) {
	for_each_tracker(
		[h](auto& tracker){
			tracker.handle_deletion_of(h);
		}
	);
}

void relational_system::create_inferred_state_for(const const_entity_handle h) {
	const auto fixtures = h.find<components::fixtures>();

	if (fixtures != nullptr) {
		ensure(!fixtures_of_bodies.is_parent_set(h));
		
		const auto owner_body = h.get_cosmos()[fixtures.get_owner_body()];
		fixtures_of_bodies.set_parent(h, owner_body);
	}
}

void relational_system::destroy_inferred_state_of(const const_entity_handle h) {
	for_each_tracker(
		[h](auto& tracker){
			tracker.unset_parents_of(h);
		}
	);
}