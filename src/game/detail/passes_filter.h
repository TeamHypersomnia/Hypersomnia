#pragma once
#include "augs/templates/maybe.h"
#include "game/detail/render_layer_filter.h"
#include "game/detail/calc_render_layer.h"

template <class E>
bool render_layer_filter::passes(const E& handle) const {
	return layers[calc_render_layer(handle)];
}

template <class C, class E>
inline bool passes_filter(
	const augs::maybe<render_layer_filter>& filter, 
	const C& cosm, 
	const E id
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
