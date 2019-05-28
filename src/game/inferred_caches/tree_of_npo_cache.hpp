#pragma once
#include "game/inferred_caches/tree_of_npo_cache.h"
#include "game/detail/calc_render_layer.h"

template <class E>
std::optional<tree_of_npo_node_input> create_default_for(const E& handle) {
	constexpr bool is_physical = E::template has<invariants::fixtures>() || E::template has<invariants::rigid_body>();
	static_assert(!is_physical);

	const auto layer = calc_render_layer(handle);

	if (const auto aabb = handle.find_aabb()) {
		tree_of_npo_node_input result;

		result.aabb = *aabb;

		if (
			layer == render_layer::UPPER_FISH 
			|| layer == render_layer::BOTTOM_FISH
			|| layer == render_layer::INSECTS
		) {
			/* Handled by the grids in organism cache */
			return std::nullopt;
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

template <class E>
void tree_of_npo_cache::specific_infer_cache_for(const E& handle) {
	const auto id = handle.get_id().to_unversioned();
	const auto it = per_entity_cache.try_emplace(id);

	auto& cache = (*it.first).second;
	const bool cache_existed = !it.second;

	if (const auto tree_node = ::create_default_for(handle)) {
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
}

