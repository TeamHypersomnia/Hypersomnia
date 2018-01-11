#include "tree_of_npo_node_component.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/components/particles_existence_component.h"
#include "game/components/transform_component.h"
#include "game/components/all_inferred_state_component.h"
#include "game/components/wandering_pixels_component.h"
#include "game/components/sound_existence_component.h"
#include "game/components/render_component.h"
#include "game/components/fixtures_component.h"
#include "augs/drawing/drawing.h"

#include "augs/ensure.h"

namespace components {
	tree_of_npo_node tree_of_npo_node::create_default_for(const const_entity_handle e) {
		tree_of_npo_node result;

		result.aabb = e.get_aabb();

		if (e.has<components::particles_existence>()) {
			result.type = tree_of_npo_type::PARTICLE_EXISTENCES;
		}
		
		return result;
	}
}

using D = components::tree_of_npo_node;

template<bool C>
bool basic_tree_of_npo_node_synchronizer<C>::is_activated() const {
	return get_raw_component().activated;
}

void component_synchronizer<false, D>::reinfer_caches() const {
	cosmic::reinfer_cache(handle.get_cosmos().get_solvable_inferred({}).tree_of_npo, handle);
}

void component_synchronizer<false, D>::update_proxy(const logic_step step) const {
	auto& sys = handle.get_cosmos().get_solvable_inferred({}).tree_of_npo;
	sys.update_proxy(handle, get_raw_component());
}

void component_synchronizer<false, D>::set_activated(const bool flag) const {
	auto& data = get_raw_component();

	if (flag == data.activated) {
		return;
	}

	data.activated = flag;
	reinfer_caches();
}

template class basic_tree_of_npo_node_synchronizer<false>;
template class basic_tree_of_npo_node_synchronizer<true>;
