#include "relational_system.h"
#include "game/components/fixtures_component.h"
#include "game/components/motor_joint_component.h"
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
	const auto motor_joint = h.find<components::motor_joint>();
	const auto& cosmos = h.get_cosmos();

	if (fixtures != nullptr && fixtures.is_activated()) {
		ensure(!fixtures_of_bodies.is_parent_set(h));
		
		const auto owner_body = fixtures.get_owner_body();
		fixtures_of_bodies.set_parent(h, owner_body);
	}

	if (motor_joint != nullptr && motor_joint.is_activated()) {
		static_assert(std::is_same_v<std::array<unversioned_entity_id, 2>, decltype(joints_of_bodies)::parent_array_type>, "Make it work for more joints plx");
		
		ensure(!joints_of_bodies.is_parent_set(h, 0));
		ensure(!joints_of_bodies.is_parent_set(h, 1));

		const auto bodies = motor_joint.get_target_bodies();

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

void relational_system::destroy_inferred_state_of(const const_entity_handle h) {
	for_each_tracker(
		[h](auto& tracker){
			tracker.unset_parents_of(h);
		}
	);
}