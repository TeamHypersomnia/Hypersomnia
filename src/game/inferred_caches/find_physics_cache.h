#pragma once
#include "game/cosmos/find_cache.h"

template <class E>
auto* find_rigid_body_cache(const E& handle) {
	return general_find_cache_dispatch<rigid_body_cache, invariants::rigid_body>(handle);
}

template <class E>
auto* find_colliders_cache(const E& handle) {
	return general_find_cache_dispatch<colliders_cache, invariants::rigid_body>(handle);
}
