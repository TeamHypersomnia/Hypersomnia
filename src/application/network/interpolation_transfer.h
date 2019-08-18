#pragma once
#include "game/cosmos/entity_pools.h"
#include "game/cosmos/pool_size_type.h"

template <class E, class T>
using entity_property_vector = std::conditional_t<
	statically_allocate_entities,
	augs::constant_size_vector<T, E::statically_allocated_entities, true>,
	std::vector<T>
>;

using all_entity_pools = per_entity_type_container<make_entity_pool>;

template <class E>
struct interpolation_transfer_cache {
	entity_property_vector<E, components::interpolation> interpolations;
	entity_property_vector<E, augs::pool_indirector<cosmic_pool_size_type>> indirectors;
};

/* template <class E> */
/* using make_interpolation_transfer_cache = interpolation_transfer_cache */

using entities_with_interpolation = entity_types_having_all_of<invariants::interpolation>;
using interpolation_transfer_caches = per_type_container<entities_with_interpolation, interpolation_transfer_cache>;

void save_interpolations(
	interpolation_transfer_caches& caches,
	const cosmos& source
);

void restore_interpolations(
	const interpolation_transfer_caches& caches,
	cosmos& target
);
