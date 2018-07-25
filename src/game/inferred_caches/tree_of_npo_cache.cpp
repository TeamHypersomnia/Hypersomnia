#include "tree_of_npo_cache.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/enums/filters.h"
#include "game/detail/calc_render_layer.h"

template <class E>
std::optional<tree_of_npo_node_input> create_default_for(const E handle) {
	const bool has_physical = 
		handle.template find<invariants::fixtures>() 
		|| handle.template has<components::rigid_body>() 
	;

	if (has_physical) {
		return std::nullopt;
	}

	const auto layer = calc_render_layer(handle);

	if (const auto aabb = handle.find_aabb()) {
		tree_of_npo_node_input result;

		result.aabb = *aabb;

		if (
			layer == render_layer::UPPER_FISH 
			|| layer == render_layer::BOTTOM_FISH
		) {
			result.type = tree_of_npo_type::ORGANISMS;
		}
		else if (layer == render_layer::CONTINUOUS_PARTICLES) {
			result.type = tree_of_npo_type::PARTICLE_STREAMS;
		}
		else if (layer == render_layer::CONTINUOUS_SOUNDS) {
			result.type = tree_of_npo_type::SOUND_SOURCES;
		}
		else if (layer == render_layer::LIGHTS) {
			result.type = tree_of_npo_type::LIGHTS;
		}
		else {
			result.type = tree_of_npo_type::RENDERABLES;
		}

		return result;
	}

	return std::nullopt;
}

tree_of_npo_cache::cache* tree_of_npo_cache::find_cache(const unversioned_entity_id id) {
	return mapped_or_nullptr(per_entity_cache, id);
}

const tree_of_npo_cache::cache* tree_of_npo_cache::find_cache(const unversioned_entity_id id) const {
	return mapped_or_nullptr(per_entity_cache, id);
}

tree_of_npo_cache::tree& tree_of_npo_cache::get_tree(const cache& c) {
	return trees[static_cast<std::size_t>(c.type)];
}

void tree_of_npo_cache::cache::clear(tree_of_npo_cache& owner) {
	if (tree_proxy_id != -1) {
		owner.get_tree(*this).nodes.DestroyProxy(tree_proxy_id);
		tree_proxy_id = -1;
	}
}

void tree_of_npo_cache::destroy_cache_of(const const_entity_handle handle) {
	const auto id = handle.get_id();

	if (const auto cache = find_cache(id)) {
		cache->clear(*this);
		per_entity_cache.erase(id);
	}
}

void tree_of_npo_cache::infer_cache_for(const const_entity_handle e) {
	e.conditional_dispatch<entities_with_render_layer>([this](const auto handle) {
		const auto id = handle.get_id().to_unversioned();
		const auto it = per_entity_cache.try_emplace(id);

		auto& cache = (*it.first).second;
		const bool cache_existed = !it.second;

		if (const auto tree_node = create_default_for(handle)) {
			const auto data = *tree_node;
			const auto new_aabb = data.aabb;

			b2AABB new_b2AABB;
			new_b2AABB.lowerBound = b2Vec2(new_aabb.left_top());
			new_b2AABB.upperBound = b2Vec2(new_aabb.right_bottom());

			const bool full_rebuild = 
				!cache_existed
				|| cache.type != data.type
			;

			if (full_rebuild) {
				cache.clear(*this);

				cache.type = data.type;
				cache.recorded_aabb = new_aabb;

				tree_of_npo_node new_node;
				new_node.payload = id;

				cache.tree_proxy_id = get_tree(cache).nodes.CreateProxy(new_b2AABB, new_node.bytes);
			}
			else {
				const vec2 displacement = new_aabb.get_center() - cache.recorded_aabb.get_center();
				get_tree(cache).nodes.MoveProxy(cache.tree_proxy_id, new_b2AABB, b2Vec2(displacement));
				cache.recorded_aabb = new_aabb;
			}
		}
		else {
			// TODO: delete cache?
		}
	});
}

void tree_of_npo_cache::reserve_caches_for_entities(const std::size_t n) {
	per_entity_cache.reserve(n);
}
