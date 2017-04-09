#include "tree_of_npo_node_component.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/components/tile_layer_instance_component.h"
#include "game/components/particles_existence_component.h"
#include "game/components/transform_component.h"
#include "game/components/inferred_state_component.h"
#include "game/components/wandering_pixels_component.h"
#include "game/components/sound_existence_component.h"
#include "game/components/render_component.h"
#include "game/components/fixtures_component.h"
#include "augs/graphics/drawers.h"

#include "game/transcendental/cosmos.h"

#include "augs/ensure.h"

namespace components {
	tree_of_npo_node tree_of_npo_node::create_default_for(const logic_step step, const_entity_handle e) {
		tree_of_npo_node result;

		const auto* const render = e.find<components::render>();

		result.aabb = e.get_aabb(step.input.metas_of_assets);

		if (e.has<components::particles_existence>()) {
			result.type = tree_of_npo_type::PARTICLE_EXISTENCES;
		}
		
		return result;
	}
}

typedef components::tree_of_npo_node D;

template<bool C>
bool basic_tree_of_npo_node_synchronizer<C>::is_activated() const {
	return component.activated;
}

void component_synchronizer<false, D>::reinference() const {
	handle.get_cosmos().partial_reinference<tree_of_npo_system>(handle);
}

void component_synchronizer<false, D>::update_proxy(const logic_step step) const {
	const auto new_aabb = components::tree_of_npo_node::create_default_for(step, handle).aabb;
	const vec2 displacement = new_aabb.center() - component.aabb.center();
	component.aabb = new_aabb;

	auto& sys = handle.get_cosmos().systems_inferred.get<tree_of_npo_system>();
	auto& cache = sys.get_cache(handle.get_id());
	
	if (cache.is_constructed() && !component.always_visible) {
		b2AABB aabb;
		aabb.lowerBound = component.aabb.left_top();
		aabb.upperBound = component.aabb.right_bottom();

		sys.get_tree(cache).nodes.MoveProxy(cache.tree_proxy_id, aabb, displacement);
	}
}

void component_synchronizer<false, D>::set_activated(bool flag) const {
	component.activated = flag;
	reinference();
}

template class basic_tree_of_npo_node_synchronizer<false>;
template class basic_tree_of_npo_node_synchronizer<true>;
