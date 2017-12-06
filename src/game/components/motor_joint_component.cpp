#include "motor_joint_component.h"

#include "augs/ensure.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/components/all_inferred_state_component.h"

typedef components::motor_joint M;

template <bool C>
bool basic_motor_joint_synchronizer<C>::is_activated() const {
	return get_raw_component().activated;
}

template <bool C>
decltype(M::target_bodies) basic_motor_joint_synchronizer<C>::get_target_bodies() const {
	return get_raw_component().target_bodies;
}

void component_synchronizer<false, M>::regenerate_caches() const {
	handle.get_cosmos().regenerate_cache<relational_cache>(handle);
	handle.get_cosmos().regenerate_cache<physics_world_cache>(handle);
}

const component_synchronizer<false, M>& component_synchronizer<false, M>::operator=(const M& m) const {
	get_raw_component() = m;
	regenerate_caches();
	return *this;
}

template class basic_motor_joint_synchronizer<false>;
template class basic_motor_joint_synchronizer<true>;