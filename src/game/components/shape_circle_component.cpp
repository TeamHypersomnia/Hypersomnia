#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/detail/physics/b2Fixture_index_in_component.h"
#include "game/components/shape_circle_component.h"
#include "game/inferred_caches/physics_world_cache.h"

template <bool C>
bool basic_shape_circle_synchronizer<C>::is_activated() const {
	return get_raw_component().activated;
}

template <bool C>
real32 basic_shape_circle_synchronizer<C>::get_radius() const {
	return get_raw_component().radius;
}

using S = components::shape_circle;

void component_synchronizer<false, S>::reinfer_caches() const {
	cosmic::reinfer_cache<physics_world_cache>(handle);
}

void component_synchronizer<false, S>::set_activated(const bool flag) const {
	if (flag == get_raw_component().activated) {
		return;
	}

	get_raw_component().activated = flag;
	reinfer_caches();
}

template class basic_shape_circle_synchronizer<false>;
template class basic_shape_circle_synchronizer<true>;