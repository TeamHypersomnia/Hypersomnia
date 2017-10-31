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

void component_synchronizer<false, D>::reinference() const {
	handle.get_cosmos().partial_reinference<tree_of_npo_system>(handle);
}

void component_synchronizer<false, D>::update_proxy(const logic_step step) const {
	const auto new_aabb = components::tree_of_npo_node::create_default_for(handle).aabb;
	auto& data = get_raw_component();

	const vec2 displacement = new_aabb.get_center() - data.aabb.get_center();
	data.aabb = new_aabb;

	auto& sys = handle.get_cosmos().inferential.tree_of_npo;
	auto& cache = sys.get_cache(handle.get_id());
	
	if (cache.is_constructed()) {
		b2AABB aabb;
		aabb.lowerBound = b2Vec2(data.aabb.left_top());
		aabb.upperBound = b2Vec2(data.aabb.right_bottom());

		sys.get_tree(cache).nodes.MoveProxy(cache.tree_proxy_id, aabb, b2Vec2(displacement));
	}
}

void component_synchronizer<false, D>::set_activated(const bool flag) const {
	auto& data = get_raw_component();

	if (flag == data.activated) {
		return;
	}

	data.activated = flag;
	reinference();
}

template class basic_tree_of_npo_node_synchronizer<false>;
template class basic_tree_of_npo_node_synchronizer<true>;
