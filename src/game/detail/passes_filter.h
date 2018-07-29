#pragma once
#include "augs/templates/maybe.h"
#include "game/detail/render_layer_filter.h"

template <class C>
inline bool passes_filter(
	const augs::maybe<render_layer_filter>& filter, 
	const C& cosm, 
	const entity_id id
) {
	if (filter.is_enabled) {
		return filter.value.passes(cosm[id]);
	}

	return true;
}

template <class E>
inline bool passes_filter(
	const augs::maybe<render_layer_filter>& filter, 
	const E& handle
) {
	if (filter.is_enabled) {
		return filter.value.passes(handle);
	}

	return true;
}
