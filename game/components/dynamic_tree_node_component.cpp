#include "dynamic_tree_node_component.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/components/tile_layer_instance_component.h"
#include "game/components/particles_existence_component.h"
#include "game/components/transform_component.h"
#include "game/components/substance_component.h"
#include "game/components/wandering_pixels_component.h"
#include "game/components/sound_existence_component.h"

#include "game/transcendental/cosmos.h"

#include "augs/ensure.h"

namespace components {
	dynamic_tree_node dynamic_tree_node::get_default(const_entity_handle e) {
		dynamic_tree_node result;

		const auto* const sprite = e.find<components::sprite>();
		const auto* const polygon = e.find<components::polygon>();
		const auto* const tile_layer_instance = e.find<components::tile_layer_instance>();
		const auto* const particles_existence = e.find<components::particles_existence>();
		const auto* const wandering_pixels = e.find<components::wandering_pixels>();
		const auto* const sound_existence = e.find<components::sound_existence>();

		if (sprite) {
			result.aabb = sprite->get_aabb(e.logic_transform());
		}
		else if (polygon) {
			result.aabb = polygon->get_aabb(e.logic_transform());
		}
		else if (tile_layer_instance) {
			result.aabb = tile_layer_instance->get_aabb(e.logic_transform());
		}
		else if (particles_existence) {
			result.type = tree_type::PARTICLE_EXISTENCES;
			result.aabb.set_position(e.logic_transform().pos);
			result.aabb.set_size({ 2.f, 2.f });

			const auto enlarge = std::max(particles_existence->input.randomize_position_within_radius, particles_existence->distribute_within_segment_of_length);
			result.aabb.expand_from_center({ enlarge, enlarge });
		}
		//else if (sound_existence) {
		//	result.type = tree_type::SOUND_EXISTENCES;
		//	result.aabb.set_position(e.logic_transform().pos);
		//
		//	const float artifacts_avoidance_epsilon = 20.f;
		//
		//	const float distance = sound_existence->calculate_max_audible_distance() + artifacts_avoidance_epsilon;
		//	result.aabb.set_size({ distance*2, distance * 2 });
		//}
		else if (wandering_pixels) {
			result.aabb = wandering_pixels->reach;

			const auto enlarge = result.aabb.get_size() * 0.3;
			result.aabb.expand_from_center(enlarge);
		}

		return result;
	}
}

typedef components::dynamic_tree_node D;

template<bool C>
bool basic_dynamic_tree_node_synchronizer<C>::is_activated() const {
	return component.activated;
}

void component_synchronizer<false, D>::resubstantiation() const {
	handle.get_cosmos().partial_resubstantiation<dynamic_tree_system>(handle);
}

void component_synchronizer<false, D>::update_proxy() const {
	const auto new_component = components::dynamic_tree_node::get_default(handle);
	const vec2 displacement = new_component.aabb.center() - component.aabb.center();
	component.aabb = new_component.aabb;

	auto& sys = handle.get_cosmos().systems_temporary.get<dynamic_tree_system>();
	auto& cache = sys.get_cache(handle.get_id());
	
	if (cache.is_constructed()) {
		b2AABB aabb;
		aabb.lowerBound = component.aabb.left_top();
		aabb.upperBound = component.aabb.right_bottom();

		sys.get_tree(cache).nodes.MoveProxy(cache.tree_proxy_id, aabb, displacement);
	}
}

void component_synchronizer<false, D>::set_activated(bool flag) const {
	component.activated = flag;
	resubstantiation();
}

template class basic_dynamic_tree_node_synchronizer<false>;
template class basic_dynamic_tree_node_synchronizer<true>;