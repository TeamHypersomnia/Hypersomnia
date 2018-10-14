#include "tree_of_npo_cache.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/enums/filters.h"
#include "game/cosmos/for_each_entity.h"
#include "game/inferred_caches/tree_of_npo_cache.hpp"

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

void tree_of_npo_cache::destroy_cache_of(const const_entity_handle& handle) {
	const auto id = handle.get_id();

	if (const auto cache = find_cache(id)) {
		cache->clear(*this);
		per_entity_cache.erase(id);
	}
}

void tree_of_npo_cache::infer_all(const cosmos& cosm) {
	cosm.for_each_entity<concerned_with>([this](const auto& handle) {
		specific_infer_cache_for(handle);
	});
}

template <class E>
constexpr bool is_npo_entity_v = tree_of_npo_cache::concerned_with<E>::value;

static_assert(!is_npo_entity_v<controlled_character>);
static_assert(!is_npo_entity_v<plain_sprited_body>);
static_assert(is_npo_entity_v<sprite_decoration>);

void tree_of_npo_cache::infer_cache_for(const const_entity_handle& e) {
	using npo_entities = entity_types_passing<concerned_with>;

	e.conditional_dispatch<npo_entities>([this](const auto& handle) {
		specific_infer_cache_for(handle);
	});
}

void tree_of_npo_cache::reserve_caches_for_entities(const std::size_t n) {
	per_entity_cache.reserve(n);
}
