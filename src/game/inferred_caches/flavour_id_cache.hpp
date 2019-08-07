#pragma once
#include "game/inferred_caches/flavour_id_cache.h"

template <class T>
void flavour_id_cache::specific_infer_cache_for(const T& typed_handle) {
	if (!enabled) {
		return;
	}

	using E = entity_type_of<T>;

	auto& m = caches.get_for<E>();
	m[typed_handle.get_flavour_id()].emplace(typed_handle.get_id());
}
