#pragma once
#include <cstddef>
#if PLATFORM_WINDOWS
#pragma warning(disable : 4503)
#endif

#include "game/inferred_caches/tree_of_npo_cache.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/inferred_caches/relational_cache.h"
#include "game/inferred_caches/flavour_id_cache.h"
#include "game/inferred_caches/processing_lists_cache.h"
#include "game/inferred_caches/organism_cache.h"

#include "game/detail/inventory/inventory_slot_id.h"

template <class T, class = void>
struct can_reserve_caches : std::false_type {};

template <class T>
struct can_reserve_caches<T, decltype(std::declval<T&>().reserve_caches_for_entities(std::size_t(1u)), void())> : std::true_type {};

template <class T>
constexpr bool can_reserve_caches_v = can_reserve_caches<T>::value;

struct cosmos_solvable_inferred {
	// GEN INTROSPECTOR struct cosmos_solvable_inferred
	relational_cache relational;
	flavour_id_cache flavour_ids;
	physics_world_cache physics;
	processing_lists_cache processing;
	tree_of_npo_cache tree_of_npo;
	organism_cache organisms;
	// END GEN INTROSPECTOR
};