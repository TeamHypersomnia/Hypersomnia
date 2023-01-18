#pragma once
#include "game/inferred_caches/tree_of_npo_cache.h"
#include "game/detail/calc_render_layer.h"
#include "game/cosmos/find_cache.h"
#include "game/inferred_caches/is_grid_organism.h"

template <class E>
auto* find_tree_of_npo_cache(const E& handle) {
	return general_find_cache<tree_of_npo_cache_data>(handle);
}

inline auto render_layer_to_tonpo_type(const render_layer layer) {
	switch (layer) {
		case render_layer::CONTINUOUS_PARTICLES:
			return tree_of_npo_type::PARTICLE_STREAMS;
		case render_layer::CONTINUOUS_SOUNDS:
			return tree_of_npo_type::SOUND_SOURCES;
		case render_layer::LIGHTS:
			return tree_of_npo_type::LIGHTS;
		case render_layer::CALLOUT_MARKERS:
			return tree_of_npo_type::CALLOUT_MARKERS;
		default:
			return tree_of_npo_type::RENDERABLES;
	}
}

template <class E>
std::optional<tree_of_npo_node_input> create_default_for(const E& handle) {
	constexpr bool is_physical = E::template has<invariants::fixtures>() || E::template has<invariants::rigid_body>();
	static_assert(!is_physical);

	const auto layer = ::calc_render_layer(handle);

	if (const auto aabb = handle.find_aabb()) {
		tree_of_npo_node_input result;

		result.aabb = *aabb;

		if (::is_grid_organism(handle)) {
			/* Handled by the grids in organism cache */
			return std::nullopt;
		}

		result.type = ::render_layer_to_tonpo_type(layer);

		return result;
	}

	return std::nullopt;
}

template <class E>
void tree_of_npo_cache::specific_infer_cache_for(const E& handle) {
	const auto id = handle.get_id().to_unversioned();

	auto& cache = get_corresponding<tree_of_npo_cache_data>(handle);
	const bool cache_existed = cache.is_constructed();

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

