#include "game/transcendental/cosmos.h"
#include "all_inferred_state_component.h"

template <bool C>
bool basic_all_inferred_state_synchronizer<C>::is_activated() const {
	return get_raw_component().activated;
}

using A = components::all_inferred_state;

void component_synchronizer<false, A>::reinfer_caches() const {
	cosmic::reinfer_caches_of(handle);
}

void component_synchronizer<false, A>::set_activated(const bool flag) const {
	if (flag == get_raw_component().activated) {
		return;
	}

	get_raw_component().activated = flag;
	reinfer_caches();
}

template class basic_all_inferred_state_synchronizer<false>;
template class basic_all_inferred_state_synchronizer<true>;