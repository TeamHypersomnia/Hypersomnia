#include "game/cosmos/logic_step.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/enums/filters.h"
#include "game/cosmos/for_each_entity.h"
#include "game/cosmos/entity_type_traits.h"
#include "game/inferred_caches/tree_of_npo_cache.hpp"

using npo_entities = entity_types_passing<tree_of_npo_cache::concerned_with>;

tree_of_npo_cache::tree& tree_of_npo_cache::get_tree(const cache& c) {
	return trees[static_cast<std::size_t>(c.type)];
}

void tree_of_npo_cache_data::clear(tree_of_npo_cache& owner) {
	if (is_constructed()) {
		owner.get_tree(*this).nodes.DestroyProxy(tree_proxy_id);
		tree_proxy_id = -1;
	}
}

void tree_of_npo_cache::destroy_cache_of(const entity_handle& e) {
	e.constrained_dispatch<npo_entities>([this](const auto& handle) {
		if (const auto cache = find_tree_of_npo_cache(handle)) {
			cache->clear(*this);
		}
	});
}

void tree_of_npo_cache::infer_all(cosmos& cosm) {
	cosm.for_each_entity<concerned_with>([this](const auto& handle) {
		specific_infer_cache_for(handle);
	});
}

template <class E>
constexpr bool is_npo_entity_v = tree_of_npo_cache::concerned_with<E>::value;

static_assert(!is_npo_entity_v<controlled_character>);
static_assert(!is_npo_entity_v<plain_sprited_body>);
static_assert(is_npo_entity_v<static_decoration>);

void tree_of_npo_cache::infer_cache_for(const entity_handle& e) {
	e.constrained_dispatch<npo_entities>([this](const auto& handle) {
		specific_infer_cache_for(handle);
	});
}

void tree_of_npo_cache::reserve_caches_for_entities(std::size_t) {

}
