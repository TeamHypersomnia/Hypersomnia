#include "game/transcendental/cosmos.h"
#include "all_inferred_state_component.h"

template <bool C>
bool basic_all_inferred_state_synchronizer<C>::is_activated() const {
	return get_data().activated;
}

using A = components::all_inferred_state;

void component_synchronizer<false, A>::reinference() const {
	handle.get_cosmos().complete_reinference(handle);
}

void component_synchronizer<false, A>::set_activated(const bool flag) const {
	if (flag == get_data().activated) {
		return;
	}

	get_data().activated = flag;
	reinference();
}

template class basic_all_inferred_state_synchronizer<false>;
template class basic_all_inferred_state_synchronizer<true>;