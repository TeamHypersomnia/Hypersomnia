#pragma once
#include <tuple>

#include "augs/misc/declare_containers.h"
#include "augs/misc/pool/pool_declaration.h"

#include "game/organization/all_entity_types.h"

#include "game/cosmos/pool_size_type.h"
#include "game/cosmos/per_entity_type.h"

template <class E>
struct entity_solvable;

template <class T>
using make_entity_pool = std::conditional_t<
	statically_allocate_entities,
	augs::pool<entity_solvable<T>, of_size<T::statically_allocated_entities>::template make_nontrivial_constant_vector, cosmic_pool_size_type, typename T::synchronized_arrays>,
	augs::pool<entity_solvable<T>, make_vector, cosmic_pool_size_type, typename T::synchronized_arrays>
>;

using all_entity_pools = per_entity_type_container<make_entity_pool>;
