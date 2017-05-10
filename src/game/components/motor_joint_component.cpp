#include "motor_joint_component.h"

#include "augs/ensure.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/components/all_inferred_state_component.h"

typedef components::motor_joint M;

template <bool C>
bool basic_motor_joint_synchronizer<C>::is_activated() const {
	return get_data().activated;
}

template <bool C>
decltype(M::target_bodies) basic_motor_joint_synchronizer<C>::get_target_bodies() const {
	return get_data().target_bodies;
}

void component_synchronizer<false, M>::reinference() const {
	handle.get_cosmos().partial_reinference<physics_system>(handle);
	handle.get_cosmos().partial_reinference<relational_system>(handle);
}

const component_synchronizer<false, M>& component_synchronizer<false, M>::operator=(const M& m) const {
	get_data() = m;
	reinference();
	return *this;
}

template class basic_motor_joint_synchronizer<false>;
template class basic_motor_joint_synchronizer<true>;