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
		return cosm[id].dispatch([&filter](const auto typed_handle) {
			return filter.value.passes(typed_handle);
		});	
	}

	return true;
}

template <class E>
inline bool passes_filter(
	const augs::maybe<render_layer_filter>& filter, 
	const E& handle
) {
	if (filter.is_enabled) {
		if constexpr(E::is_specific) {
			return filter.value.passes(handle);
		}
		else {
			return handle.dispatch([&filter](const auto typed_handle) {
				return filter.value.passes(typed_handle);
			});	
		}
	}

	return true;
}
