#include "dynamic_tree_node_component.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/components/tile_layer_instance_component.h"
#include "game/components/particles_existence_component.h"
#include "game/components/transform_component.h"
#include "game/components/inferred_state_component.h"
#include "game/components/wandering_pixels_component.h"
#include "game/components/sound_existence_component.h"

#include "game/transcendental/cosmos.h"

#include "augs/ensure.h"

namespace components {
	dynamic_tree_node dynamic_tree_node::get_default(const_entity_handle e) {
		dynamic_tree_node result;

		const auto* const render = e.find<components::render>();

		if (render && render->screen_space_transform) {
			result.always_visible = true;
			return result;
		}

		result.aabb = e.get_aabb();

		if (e.has<components::particles_existence>()) {
			result.type = tree_type::PARTICLE_EXISTENCES;
		}
		
		return result;
	}
}

typedef components::dynamic_tree_node D;

template<bool C>
bool basic_dynamic_tree_node_synchronizer<C>::is_activated() const {
	return component.activated;
}

void component_synchronizer<false, D>::reinference() const {
	handle.get_cosmos().partial_reinference<dynamic_tree_system>(handle);
}

void component_synchronizer<false, D>::update_proxy() const {
	const auto new_aabb = components::dynamic_tree_node::get_default(handle).aabb;
	const vec2 displacement = new_aabb.center() - component.aabb.center();
	component.aabb = new_aabb;

	auto& sys = handle.get_cosmos().systems_inferred.get<dynamic_tree_system>();
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

template class basic_dynamic_tree_node_synchronizer<false>;
template class basic_dynamic_tree_node_synchronizer<true>;