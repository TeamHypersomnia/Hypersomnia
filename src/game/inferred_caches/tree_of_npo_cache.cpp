#include "tree_of_npo_cache.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/enums/filters.h"

std::optional<tree_of_npo_node_input> tree_of_npo_node_input::create_default_for(const const_entity_handle e) {
	const bool has_renderable = 
		e.find<components::render>() 
		|| e.has<components::particles_existence>()
		//|| has<components::sound_existence>()
	;

	const bool has_physical = 
		e.has<components::fixtures>() 
		|| e.has<components::rigid_body>() 
	;

	if (has_renderable && !has_physical) {
		tree_of_npo_node_input result;

		result.aabb = e.get_aabb();

		if (e.has<components::particles_existence>()) {
			if (e.get<components::processing>().is_in(processing_subjects::WITH_PARTICLES_EXISTENCE)) {
				result.type = tree_of_npo_type::PARTICLE_EXISTENCES;
			}
			else {
				return std::nullopt;		
			}
		}

		return result;
	}

	return std::nullopt;
}

bool tree_of_npo_cache::cache::is_constructed() const {
	return constructed;
}

tree_of_npo_cache::cache& tree_of_npo_cache::get_cache(const unversioned_entity_id id) {
	return per_entity_cache[linear_cache_key(id)];
}

const tree_of_npo_cache::cache& tree_of_npo_cache::get_cache(const unversioned_entity_id id) const {
	return per_entity_cache[linear_cache_key(id)];
}

tree_of_npo_cache::tree& tree_of_npo_cache::get_tree(const cache& c) {
	return trees[static_cast<size_t>(c.type)];
}

bool tree_of_npo_cache::is_tree_node_constructed_for(const entity_id id) const {
	return get_cache(id).is_constructed();
}

void tree_of_npo_cache::destroy_cache_of(const const_entity_handle handle) {
	auto& cache = get_cache(handle.get_id());

	if (cache.is_constructed()) {
		if (cache.tree_proxy_id != -1) {
			get_tree(cache).nodes.DestroyProxy(cache.tree_proxy_id);
		}

		cache = tree_of_npo_cache::cache();
	}
}

void tree_of_npo_cache::infer_cache_for(const const_entity_handle handle) {
	auto& cache = get_cache(handle.get_id());

	if (const auto tree_node = tree_of_npo_node_input::create_default_for(handle)) {
		const auto data = *tree_node;
		const auto new_aabb = data.aabb;

		b2AABB new_b2AABB;
		new_b2AABB.lowerBound = b2Vec2(new_aabb.left_top());
		new_b2AABB.upperBound = b2Vec2(new_aabb.right_bottom());

		const bool full_rebuild = 
			!cache.is_constructed()
			|| cache.type != data.type
		;
		
		if (full_rebuild) {
			destroy_cache_of(handle);
			
			cache.type = data.type;
			cache.recorded_aabb = new_aabb;

			tree_of_npo_node new_node;
			new_node.payload = handle.get_id().operator unversioned_entity_id();

			cache.tree_proxy_id = get_tree(cache).nodes.CreateProxy(new_b2AABB, new_node.bytes);

			cache.constructed = true;
		}
		else {
			const vec2 displacement = new_aabb.get_center() - cache.recorded_aabb.get_center();
			get_tree(cache).nodes.MoveProxy(cache.tree_proxy_id, new_b2AABB, b2Vec2(displacement));
			cache.recorded_aabb = new_aabb;
		}
	}
}

void tree_of_npo_cache::reserve_caches_for_entities(const size_t n) {
	per_entity_cache.resize(n);
}
